#include "scheduler.hpp"

void FCFSSched::add_process(Process* proc) {
    readyQ.push(proc);
}

Process* FCFSSched::get_next_process() {
    if(readyQ.empty())
        return nullptr;

    Process* ret = readyQ.front();
    readyQ.pop();
    return ret;
}

void LCFSSched::add_process(Process* proc) {
    readyQ.push(proc);
}

Process* LCFSSched::get_next_process() {
    if(readyQ.empty())
        return nullptr;

    Process* ret = readyQ.top();
    readyQ.pop();
    return ret;
}

// find first process in readyq with remaining time greater
// than proc and insert before it.
void SRTFSched::add_process(Process* proc) {
    if(readyQ.empty()) {
        readyQ.push_back(proc);
        return;
    }
    // find process with rt > proc
    std::list<Process*>::iterator it = readyQ.begin();
    int curr_rt = proc->getRemTime();
    while(it != readyQ.end() && (*it)->getRemTime() <= curr_rt) {
        it++;
    }
    readyQ.insert(it, proc);
}

Process* SRTFSched::get_next_process() {
    if(readyQ.empty())
        return nullptr;

    Process* ret = readyQ.front();
    readyQ.pop_front();
    return ret;
}

void PRIOSched::add_process(Process* proc) {
    int idx = proc->getPrio();
    if(idx >= 0) {
        activeQ[idx].push(proc);
    }
    else {
        proc->setDynamicPrio(proc->getStPrio()-1);
        idx = proc->getPrio();
        expiredQ[idx].push(proc);
    }
}

Process* PRIOSched::get_next_process() {
    int idx = maxprios - 1;
    Process* ret;
    while(idx != -1 && activeQ[idx].empty()){
        idx--;
    }
    if(idx >= 0) {
        ret = activeQ[idx].front();
        activeQ[idx].pop();
        return ret;
    }

    activeQ.swap(expiredQ);
    
    idx = maxprios - 1;
    while(idx != -1 && activeQ[idx].empty()) {
        idx--;
    }
    if(idx >= 0) {
        ret = activeQ[idx].front();
        activeQ[idx].pop();
        return ret;
    }
    return nullptr;
}

bool PRIOSched::test_preempt(Process* curr_proc, Process* unblocked, int current_ts, Event* exit_event) {
    
    if(!eflag || exit_event->getEvtTimestamp() == current_ts || !exit_event->isEvtValid())
        return false;

    int idx;

    for(idx = maxprios - 1; idx > curr_proc->getPrio() && activeQ[idx].empty(); idx--);
    
    return unblocked->getPrio() > curr_proc->getPrio() && idx > curr_proc->getPrio();
}
