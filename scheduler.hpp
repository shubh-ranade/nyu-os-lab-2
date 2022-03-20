#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <iostream>
#include <queue>
#include "process.hpp"

class Scheduler {
public:
    virtual void add_process(Process* proc) = 0;
    virtual Process* get_next_process() = 0;
    virtual ~Scheduler() = default;
};

class FIFOSched : public Scheduler {
private:
    std::queue<Process*> readyQ;
public:
    virtual void add_process(Process* proc);
    virtual Process* get_next_process();
    // virtual ~FIFOSched() {
    //     delete &readyQ;
    // }
};

#endif
