#include "scheduler.hpp"

void FIFOSched::add_process(Process* proc) {
    readyQ.push(proc);
}

Process* FIFOSched::get_next_process() {
    if(readyQ.empty())
        return nullptr;

    Process* ret = readyQ.front();
    readyQ.pop();
    return ret;
}
