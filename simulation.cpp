#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fstream>
#include "scheduler.hpp"
#include "event.hpp"

using namespace std;

int ofs = 0;

const char* transition_names[] = {
    "CREATED -> READY",
    "READY -> RUNNG",
    "RUNNG -> BLOCK",
    "BLOCK -> READY",
    "RUNNG -> READY"
};

class Compare {
public:
    bool operator() (Event* lhs, Event* rhs) {
        return lhs->getEvtTimestamp() > rhs->getEvtTimestamp();
    }
};

ostream& operator<<(ostream& os, Process& proc) {
    return os << "Pid: " << proc.getPID() << " AT: " << proc.getAT() << " Rem T: " << proc.getRemTime()
        << " IOT: " << proc.getIOT() << " CWT: " << proc.getCWT() << " State: " << proc.getState()
        << " FT: " << proc.getFT();
}

ostream& operator<<(ostream& os, Event& evt) {
    return os << "Timestamp: " << evt.getEvtTimestamp() << " Old state: " << evt.getOldState()
        << " New state: " << evt.getNewState() << " Process Info: " << *evt.getEvtProc()
        << " Time in prev state: " << evt.getEvtTimestamp() - evt.getEvtProc()->getStateTS();
}

class Simulation {
private:
    std::priority_queue<Event*, std::deque<Event*>, Compare> eventQ;
    int total_sim_time;
    double cpu_eff, io_eff, avg_tt, avg_wt, throughput;

public:
    Simulation(std::priority_queue<Event*, std::deque<Event*>, Compare>& q): eventQ(q) {}
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
        throughput = (n * 100.0) / max_ft;
        avg_tt = (double)acc_tt / n;
        avg_wt = (double)acc_wt / n;

        printf("SUM: %d %.2lf %.2lf %.2lf %.2lf %.3lf\n", total_sim_time, cpu_eff, io_eff, avg_tt, avg_wt, throughput);
    }
};

int getrandom(int burst, int* randvals, int randvals_c) {
    ofs = ofs % randvals_c;
    // cout << "in getrandom ofs = " << ofs << " randvals_c = " << randvals_c << " num " << randvals[ofs]
    // << " burst " << burst << '\n';
    return 1 + (randvals[ofs++] % burst);
}

priority_queue<Event*, deque<Event*>, Compare> parseInput(ifstream& f, int maxprio, int* randvals, int randvals_c) {
    int at, tc, cb, io;
    priority_queue<Event*, deque<Event*>, Compare> pq;
    while(f >> at >> tc >> cb >> io) {
        // pid is the same as current size of priority queue
        Process* newProc = new Process(pq.size(), at, tc, cb, io, CREATED, getrandom(maxprio, randvals, randvals_c));
        Event* evt = new Event(at, newProc, CREATED, READY, CREATED_TO_READY);
        pq.push(evt);
    }

    return pq;
}

int main(int argc, char **argv) {
    
    //TODO: handle arguments with getopt
    if(!argv[1] || !argv[2])
        return 1;

    ifstream randin;
    randin.open(argv[2]);
    int randvals_c;
    randin >> randvals_c;
    int randvals[randvals_c];
    for(int i = 0; i < randvals_c; i++) {
        randin >> randvals[i];
    }
    // cout << "rand count " << randvals_c << '\n';
    randin.close();

    ifstream input;
    input.open(argv[1]);
    int quantum = INT32_MAX, maxprio = 4;

    priority_queue<Event*, deque<Event*>, Compare> event_queue = parseInput(input, maxprio, randvals, randvals_c);
    input.close();

    // assign priorities to processes

    // at exit, store processes here (process exits when rem_t = 0)
    vector<Process*> processes(event_queue.size());

    // TODO: make dynamic, initialize properly
    Scheduler* scheduler = new FIFOSched();

    Simulation* sim = new Simulation(event_queue);
    Event* evt;
    int current_time = 0;
    Process* current_running_process = nullptr;

    while( (evt = sim->getEvent()) ) {
        // cout << *evt << '\n';
        // cout << evt->getTransition() << '\n';
        Process* proc = evt->getEvtProc();
        current_time = evt->getEvtTimestamp();
        state_transition_t transition = evt->getTransition();
        int time_in_prev_state = current_time - proc->getStateTS();
        proc->setStateTS(current_time);
        bool call_scheduler = false;
        delete evt; evt = nullptr;

        switch (transition) {
        case CREATED_TO_READY: // process created, ready to run at the same time
        case BLOCKED_TO_READY:
        case RUN_TO_READY:
        {
            if(proc->getState() == RUNNING)
                proc->setRemTime(proc->getRemTime() - time_in_prev_state);
            
            printf("%d %d %d: %s\n", current_time, proc->getPID(), time_in_prev_state, transition_names[transition]);
            proc->setState(READY);
            // cout << "pushing proc " << *proc << '\n';
            scheduler->add_process(proc);
            call_scheduler = true;
            break;
        }

        case READY_TO_RUN: {
            // create event for preemption or blocking, no call to scheduler
            proc->setState(RUNNING);
            current_running_process = proc;
            int current_cb = min(getrandom(proc->getCB(), randvals, randvals_c), proc->getRemTime());
            printf("%d %d %d: %s cb=%d rem=%d prio=%d\n", current_time, proc->getPID(), time_in_prev_state, transition_names[transition], current_cb, proc->getRemTime(), proc->getPrio());
            // cout << "cb = " << current_cb << " running proc " << *current_running_process << '\n';
            int run_proc_time;
            if(current_cb <= quantum) {
                run_proc_time = min(current_cb, proc->getRemTime());
                sim->putEvent(new Event(current_time + run_proc_time, current_running_process,
                                            RUNNING, BLOCKED, RUN_TO_BLOCKED));
            } else {
                if(quantum < proc->getRemTime()) {
                    run_proc_time = quantum;
                    sim->putEvent(new Event(current_time + run_proc_time, current_running_process,
                                                RUNNING, READY, RUN_TO_READY));
                }
                else { // process exits before quantum
                    run_proc_time = proc->getRemTime();
                    sim->putEvent(new Event(current_time + run_proc_time, current_running_process,
                                                RUNNING, BLOCKED, RUN_TO_BLOCKED));
                }
            }
            break;
        }
        
        case RUN_TO_BLOCKED: { // all processes exit in blocked state
            proc->setState(BLOCKED);
            proc->setRemTime(proc->getRemTime() - time_in_prev_state);
            call_scheduler = true;
            
            // if process has run for tc, don't do io, add process to final vector
            // TODO: calculate stats
            if(proc->getRemTime() == 0) {
                // cout << "exiting proc " << *proc << '\n';
                proc->setFT(current_time);
                proc->setTT(current_time - proc->getAT());
                processes[proc->getPID()] = proc;
                printf("%d %d %d: Done\n", current_time, proc->getPID(), time_in_prev_state);
                current_running_process = nullptr;
                break;
            }
            
            int io_time = getrandom(proc->getIO(), randvals, randvals_c);
            printf("%d %d %d: %s  ib=%d rem=%d\n", current_time, proc->getPID(), time_in_prev_state, transition_names[transition], io_time, proc->getRemTime());
            // cout << "io_time = " << io_time << " blocking proc " << *proc << '\n';
            proc->setIOT(proc->getIOT() + io_time);
            sim->putEvent(new Event(current_time + io_time, current_running_process, BLOCKED, READY, BLOCKED_TO_READY));
            current_running_process = nullptr;
            break;
        }
        
        default: {
            cout << "Invalid transition " << transition << " panic!!\n";
            exit(1);
            break;
        }
        } // switch ends

        if(call_scheduler) {
            if(sim->getNextEventTime() == current_time) {
                continue; //process next event from event queue WHY??
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
                sim->putEvent(new Event(current_time, current_running_process, READY, RUNNING, READY_TO_RUN));
            }
        }
    }

    sim->calculate_metrics(processes);

    return EXIT_SUCCESS;
}
