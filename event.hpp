#ifndef EVENT_HPP
#define EVENT_HPP
#include "process.hpp"

class Event {
private:
    int evtTimestamp;
    process_state_t oldState, newState;
    state_transition_t transition;
    Process* evtProc;

public:
    Event(int ts, Process* proc, process_state_t os, process_state_t ns, state_transition_t trans)
        : evtTimestamp(ts)
        , evtProc(proc)
        , oldState(os)
        , newState(ns)
        , transition(trans)
    {}

    int getEvtTimestamp() { return evtTimestamp; }
    Process* getEvtProc() { return evtProc; }
    process_state_t getOldState() { return oldState; }
    process_state_t getNewState() { return newState; }
    state_transition_t getTransition() { return transition; }
};

#endif