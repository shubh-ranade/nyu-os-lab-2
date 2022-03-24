#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include "scheduler.hpp"
#include "event.hpp"

using namespace std;

int ofs = 0;
int genTimestamp = 0;

const char* transition_names[] = {
    "CREATED -> READY",
    "READY -> RUNNG",
    "RUNNG -> BLOCK",
    "BLOCK -> READY",
    "RUNNG -> READY"
};

const char* prio_test_preempt_action[] = {
	"NO",
	"YES"
};

class EvtCompare {
public:
    bool operator() (Event* lhs, Event* rhs) {
        if(lhs->getEvtTimestamp() == rhs->getEvtTimestamp()) {
            return lhs->getGenTimestamp() > rhs->getGenTimestamp();
        }
        return lhs->getEvtTimestamp() > rhs->getEvtTimestamp();
    }
};

ostream& operator<<(ostream& os, Process& proc) {
    return os << "Pid: " << proc.getPID() << " AT: " << proc.getAT() << " Rem T: " << proc.getRemTime()
        << " IOT: " << proc.getIOT() << " CWT: " << proc.getCWT() << " State: " << proc.getState()
        << " FT: " << proc.getFT();
}

ostream& operator<<(ostream& os, Event& evt) {
    return os << "Timestamp: " << evt.getEvtTimestamp() << " Gen Timestamp: " << evt.getGenTimestamp() << " Old state: " << evt.getOldState()
        << " New state: " << evt.getNewState() << " Process Info: " << *evt.getEvtProc()
        << " Time in prev state: " << evt.getEvtTimestamp() - evt.getEvtProc()->getStateTS();
}

typedef priority_queue<Event*, vector<Event*>, EvtCompare> des_queue;

class Simulation {
private:
    des_queue eventQ;
    int total_sim_time;
	int acc_io_time, num_proc_in_io, io_start, io_end;
    double cpu_eff, io_eff, avg_tt, avg_wt, throughput;

public:
    Simulation(des_queue& q): eventQ(q), acc_io_time(0), num_proc_in_io(0), io_start(0), io_end(0) {}
	int getIOTime() { return acc_io_time; }
	void setIOTime(int t) { acc_io_time = t; }
    // bool hasMoreEvents() { return !eventQ.empty(); }
    Event* getEvent() {
        Event* ret = nullptr;
        if(!eventQ.empty()) {
            ret = eventQ.top();
            eventQ.pop();
        }
        return ret;
    }

    void putEvent(Event* evt) { eventQ.push(evt); }
    void rmEvent() { return; }
    int getNextEventTime() { return eventQ.empty() ? -1 : eventQ.top()->getEvtTimestamp(); }
	bool isNextEventValid() { return eventQ.top()->isEvtValid(); }

	void io_interval_start(int current_ts) {
		num_proc_in_io++;
		if(num_proc_in_io == 1) {
			io_start = current_ts;
		}
	}

	void io_interval_end(int current_ts) {
		num_proc_in_io--;
		if(num_proc_in_io == 0) {
			io_end = current_ts;
			acc_io_time += + io_end - io_start;
		}
	}
    
    void calculate_metrics(vector<Process*> processes) {
        int max_ft = -1; // equal to total_sim_time
        int acc_cpu_time = 0;
        int acc_tt = 0, acc_wt = 0;
        double n = processes.size();
        for(Process* proc : processes) {
            max_ft = max(proc->getFT(), max_ft);
            acc_cpu_time += proc->getTT() - proc->getCWT() - proc->getIOT();
            acc_tt += proc->getTT();
            acc_wt += proc->getCWT();
            printf("%04d: %4d %4d %4d %4d %1d | %5d %5d %5d %5d\n", 
                    proc->getPID(), proc->getAT(), proc->getTC(), proc->getCB(), proc->getIO(),
                    proc->getStPrio(), proc->getFT(), proc->getTT(), proc->getIOT(), proc->getCWT());
        }
        total_sim_time = max_ft;
        cpu_eff = ((double)acc_cpu_time*100) / max_ft;
		io_eff = ((double)acc_io_time*100) / max_ft;
        throughput = (n * 100.0) / max_ft;
        avg_tt = (double)acc_tt / n;
        avg_wt = (double)acc_wt / n;

        printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", total_sim_time, cpu_eff, io_eff, avg_tt, avg_wt, throughput);
    }
};

int getrandom(int burst, int* randvals, int randvals_c) {
    ofs = ofs % randvals_c;
    return 1 + (randvals[ofs++] % burst);
}

des_queue parseInput(ifstream& f, int maxprio, int* randvals, int randvals_c) {
    int at, tc, cb, io;
    des_queue pq;
    while(f >> at >> tc >> cb >> io) {
        // pid is the same as current size of priority queue
        Process* newProc = new Process(pq.size(), at, tc, cb, io, CREATED, getrandom(maxprio, randvals, randvals_c));
        Event* evt = new Event(at, genTimestamp++, newProc, CREATED, READY, CREATED_TO_READY);
        pq.push(evt);
    }

    return pq;
}

//TODO: handle events with same timestamps!!!
int main(int argc, char **argv) {
    
    // handle arguments with getopt
    bool verbose = false, trace = false, eflag = false, pflag = false;
	char *schedspec = NULL;
	int index;
	int c;
	char sched_type;
	int q_arg, mprio_arg;
    int quantum = INT32_MAX, maxprio = 4;

	opterr = 0;

	while ((c = getopt (argc, argv, "vteps:")) != -1)
	switch (c)
    {
    case 'v':
        verbose = true;
        break;
    case 't':
        trace = true;
        break;
    case 'e':
        eflag = true;
        break;
    case 'p':
        pflag = true;
        break;
    case 's':
    {
        schedspec = optarg;
        int num_read = sscanf(schedspec, "%c%d:%d", &sched_type, &q_arg, &mprio_arg);
        switch (num_read)
        {
        case 3: {
            quantum = q_arg;
            maxprio = mprio_arg;
            break;
		}
        
        case 2:
            quantum = q_arg;
            break;
        
		case 1:
			break;

		default:
            printf("No scheduler specified\n");
			exit(EXIT_FAILURE);
        }
		if(sched_type != 'F' && sched_type != 'L' && sched_type != 'S' && sched_type != 'R' && sched_type != 'P' && sched_type != 'E') {
			printf("Unknown arguemnt %c with -%c option", sched_type, c);
			exit(EXIT_FAILURE);
		}
        break;
    }
    case '?':
        if (optopt == 's')
            fprintf (stderr, "Option -%c requires an argument.\n", optopt);
        else if (isprint (optopt))
            fprintf (stderr, "Unknown option `-%c'.\n", optopt);
        else
            fprintf (stderr,
                    "Unknown option character `\\x%x'.\n",
                    optopt);
        return 1;
    default:
		exit(EXIT_FAILURE);
    }

    if(!argv[optind] || !argv[optind+1]) {
		printf("Not enough non-optional arguments. Got %s and %s.\n", argv[optind], argv[optind+1]);
		printf("Usage: <program> [-v] [-t] [-e] [-p] [-s [FLS | R<num> | P<num>[:<maxprio>] | E<num>[:<maxprios>]]] inputfile randfile\n");
	}

    // read randfile
	ifstream randin;
    randin.open(argv[optind+1]);
    int randvals_c;
    randin >> randvals_c;
    int randvals[randvals_c];
    for(int i = 0; i < randvals_c; i++) {
        randin >> randvals[i];
	}
    randin.close();

	// read inputfile
    ifstream input;
    input.open(argv[optind]);
    des_queue event_queue = parseInput(input, maxprio, randvals, randvals_c);
    input.close();

    // at exit, store processes here (process exits when rem_t = 0)
    vector<Process*> processes(event_queue.size());

    // create scheduler
    Scheduler* scheduler;
	switch (sched_type)
	{
	case 'F':
		scheduler = new FCFSSched();
		break;

	case 'R':
		scheduler = new FCFSSched();
		break;
	
	case 'L':
		scheduler = new LCFSSched();
		break;

	case 'S':
		scheduler = new SRTFSched();
		break;
	
	case 'P':
		scheduler = new PRIOSched(maxprio, false);
		break;

	case 'E':
		scheduler = new PRIOSched(maxprio, true);
		break;

	default: {
		printf("Unknown scheduler\n");
		exit(EXIT_FAILURE);
	}
	}

    Simulation* sim = new Simulation(event_queue);
    Event* evt;
    int current_time = 0;
	int num_proc_in_io = 0, io_start = 0, io_end = 0;
    Process* current_running_process = nullptr;
	Event* exit_event = nullptr;

    // simulation
    while( (evt = sim->getEvent()) ) {
		if(!evt->isEvtValid()) {
			delete evt; evt = nullptr;
			continue;
		}

        Process* proc = evt->getEvtProc();
        current_time = evt->getEvtTimestamp();
        state_transition_t transition = evt->getTransition();
        int time_in_prev_state = current_time - proc->getStateTS();
        proc->setStateTS(current_time);
        bool call_scheduler = false;
		bool prio_preemption = false;
        delete evt; evt = nullptr;

        switch (transition) {
        case CREATED_TO_READY:
        case BLOCKED_TO_READY:
        case RUN_TO_READY: {
            if(proc->getState() == RUNNING) { // if previous state was running
            	// calculate new remaining time as well as remaining cb
                proc->setRemTime(proc->getRemTime() - time_in_prev_state);
                proc->setRemCB(proc->getRemCB() - time_in_prev_state);
				current_running_process = nullptr;
            }

			// measure io util if prev state was blocked
			if(proc->getState() == BLOCKED) {
				proc->setDynamicPrio(proc->getStPrio()-1);
				sim->io_interval_end(current_time);
			}
            
            if(verbose) {
				if(proc->getState() == RUNNING) {
					printf("%d %d %d: %s  cb=%d rem=%d prio=%d\n", current_time, proc->getPID(),
						time_in_prev_state, transition_names[transition], 
						proc->getRemCB(), proc->getRemTime(), proc->getPrio());
				}
				else
					printf("%d %d %d: %s\n", current_time, proc->getPID(), time_in_prev_state, transition_names[transition]);
			}

			if(proc->getState() == RUNNING && (sched_type == 'P' || sched_type == 'E'))
				proc->setDynamicPrio(proc->getPrio()-1);
			proc->setState(READY);
            scheduler->add_process(proc);
            call_scheduler = true;
            break;
        }

        case READY_TO_RUN: {
            // create event for preemption or blocking, no call to scheduler
            proc->setState(RUNNING);
            current_running_process = proc;
            int proc_rem_time = proc->getRemTime();
            
            // new cb only if current cb has expired
            int current_cb = proc->getRemCB() == 0
                            ? min(getrandom(proc->getCB(), randvals, randvals_c), proc_rem_time)
                            : proc->getRemCB();
            
            if(verbose)
			printf("%d %d %d: %s cb=%d rem=%d prio=%d\n", current_time, proc->getPID(),
                    time_in_prev_state, transition_names[transition], 
                    current_cb, proc_rem_time, proc->getPrio());
            
            proc->setRemCB(current_cb);

            int run_proc_time;
            run_proc_time = min(current_cb, min(proc_rem_time, quantum));
			Event* exit_current_running_proc;

            // check for termination; process exits in blocked state
			if(run_proc_time == proc_rem_time) {
                exit_current_running_proc = new Event(current_time + run_proc_time, genTimestamp++, proc,
                                        RUNNING, BLOCKED, RUN_TO_BLOCKED);
			} else if(run_proc_time == current_cb) {  // check for io scheduling
                exit_current_running_proc = new Event(current_time + run_proc_time, genTimestamp++, proc,
                                        RUNNING, BLOCKED, RUN_TO_BLOCKED);
            } else if(run_proc_time == quantum) { // check for quantum expiration
				exit_current_running_proc = new Event(current_time + run_proc_time, genTimestamp++, proc,
                                        RUNNING, READY, RUN_TO_READY);
            }
			sim->putEvent(exit_current_running_proc);
			exit_event = exit_current_running_proc;
            break;
        }
        
        case RUN_TO_BLOCKED: { // all processes exit in blocked state
            proc->setState(BLOCKED);
            proc->setRemTime(proc->getRemTime() - time_in_prev_state);
            proc->setRemCB(0);
			sim->io_interval_start(current_time);
            call_scheduler = true;
            
            // if process has run for tc, don't do io, add process to final vector
            if(proc->getRemTime() == 0) {
				sim->io_interval_end(current_time);
                proc->setFT(current_time);
                proc->setTT(current_time - proc->getAT());
                processes[proc->getPID()] = proc;
                if(verbose)
					printf("%d %d %d: Done\n", current_time, proc->getPID(), time_in_prev_state);
                current_running_process = nullptr;
                break;
            }
            
            int io_time = getrandom(proc->getIO(), randvals, randvals_c);
            if(verbose)
			printf("%d %d %d: %s  ib=%d rem=%d\n", current_time, proc->getPID(),
                    time_in_prev_state, transition_names[transition], io_time, proc->getRemTime());
            proc->setIOT(proc->getIOT() + io_time);
            sim->putEvent(new Event(current_time + io_time, genTimestamp++, proc, BLOCKED, READY, BLOCKED_TO_READY));
            current_running_process = nullptr;
            break;
        }
        
        default: {
			printf("Invalid transition %s, panic!!\n", transition_names[transition]);
            exit(1);
            break;
        }
        } // switch ends

        if(call_scheduler) {
			// if there is some process running, test if it should be preempted
			// if it should be preempted, invalidate current exit event and create new exit event with the same 
			// timestamp as current time.
			if(current_running_process && sched_type == 'E') {
				prio_preemption = scheduler->test_preempt(current_running_process, proc, current_time, exit_event);
//				if(verbose)
//				printf("---> PRIO preemption %d by %d ? %d TS=%d now=%d) --> %s\n",
//						current_running_process->getPID(), proc->getPID(), proc->getPrio() > current_running_process->getPrio(),
//						exit_event->getEvtTimestamp(), current_time, prio_test_preempt_action[(int)prio_preemption]);
				if(prio_preemption) {
					exit_event->invalidate();
					exit_event = new Event(current_time, genTimestamp++, current_running_process, RUNNING, READY, RUN_TO_READY);
					sim->putEvent(exit_event);
				}
			}
            
			if(sim->getNextEventTime() == current_time && sim->isNextEventValid()) {
                continue; //process next event from event queue
            }
			
            call_scheduler = false;
            if(current_running_process == nullptr) {
                current_running_process = scheduler->get_next_process();
                if(current_running_process == nullptr)
                    continue; //no processes to schedule, wait till next event
                
                // increment cpu waiting time
                int time_in_ready_queue = current_time - current_running_process->getStateTS();
                current_running_process->setCWT(current_running_process->getCWT() + time_in_ready_queue);

                // create event to make the process runnable
                sim->putEvent(new Event(current_time, genTimestamp++, current_running_process, READY, RUNNING, READY_TO_RUN));
            }
        }
    }

	switch (sched_type)
	{
	case 'F':
		printf("FCFS\n");
		break;

	case 'R':
		printf("RR %d\n", quantum);
		break;
	
	case 'L':
		printf("LCFS\n");
		break;

	case 'S':
		printf("SRTF\n");
		break;
	
	case 'P':
		printf("PRIO %d\n", quantum);
		break;

	case 'E':
		printf("PREPRIO %d\n", quantum);
		break;
	
	default: {
		printf("Unknown scheduler\n");
		exit(EXIT_FAILURE);
	}
	}

    sim->calculate_metrics(processes);

    delete sim;

    return EXIT_SUCCESS;
}
