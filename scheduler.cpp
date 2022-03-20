#include "scheduler.hpp"

void FIFOSched::add_process(Process* proc) {
    // std::cout << " pushing proc " << proc << '\n';
    readyQ.push(proc);
}

Process* FIFOSched::get_next_process() {
    // std::cout << "y\n";
    if(readyQ.empty())
        return nullptr;

    Process* ret = readyQ.front();
    readyQ.pop();
    return ret;
}
