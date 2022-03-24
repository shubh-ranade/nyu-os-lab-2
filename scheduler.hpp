#ifndef SCHEDULER_HPP
#define SCHEDULER_HPP
#include <iostream>
#include <list>
#include <queue>
#include <stack>
#include "event.hpp"

class Scheduler {
public:
    virtual void add_process(Process* proc) = 0;
    virtual Process* get_next_process() = 0;
    virtual bool test_preempt(Process* curr_proc, Process* unblocked, int current_ts, Event* exit_event) { return false; }
    virtual ~Scheduler() = default;
};

class FCFSSched : public Scheduler {
private:
    std::queue<Process*> readyQ;
public:
    virtual void add_process(Process* proc);
    virtual Process* get_next_process();
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
    virtual ~SRTFSched() {
        delete &readyQ;
    }
};

class PRIOSched : public Scheduler {
private:
    int maxprios;
    bool eflag;
    std::vector<std::queue<Process*>> activeQ;
    std::vector<std::queue<Process*>> expiredQ;
public:
    virtual void add_process(Process* proc);
    virtual Process* get_next_process();
    virtual bool test_preempt(Process* curr_proc, Process* unblocked, int current_ts, Event* exit_event);
    PRIOSched(int maxprio, bool e) : maxprios(maxprio), activeQ(maxprio), expiredQ(maxprio), eflag(e) {}
    virtual ~PRIOSched() {
        delete &activeQ;
        delete &expiredQ;
    }
};

#endif
