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
