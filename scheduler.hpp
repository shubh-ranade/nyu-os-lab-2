#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <iostream>
#include <list>
#include <queue>
#include <stack>
#include "process.hpp"

class Scheduler {
public:
    virtual void add_process(Process* proc) = 0;
    virtual Process* get_next_process() = 0;
    virtual bool test_preempt(Process* proc, int current_time) = 0;
    virtual ~Scheduler() = default;
};

class FCFSSched : public Scheduler {
private:
    std::queue<Process*> readyQ;
public:
    virtual void add_process(Process* proc);
    virtual Process* get_next_process();
    virtual bool test_preempt(Process* proc, int current_time) {}
    virtual ~FCFSSched() {
        delete &readyQ;
    }
};

class LCFSSched : public Scheduler {
private:
    std::stack<Process*> readyQ;
public:
    virtual void add_process(Process* proc);
    virtual Process* get_next_process();
    virtual bool test_preempt(Process* proc, int current_time) {}
    virtual ~LCFSSched() {
        delete &readyQ;
    }
};

class SRTFSched : public Scheduler {
private:
    std::list<Process*> readyQ;
public:
    virtual void add_process(Process* proc);
    virtual Process* get_next_process();
    virtual bool test_preempt(Process* proc, int current_time) {}
    virtual ~SRTFSched() {
        delete &readyQ;
    }
};

#endif
