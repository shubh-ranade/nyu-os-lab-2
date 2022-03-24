#ifndef EVENT_HPP
#define EVENT_HPP
#include "process.hpp"

class Event {
private:
    int evtTimestamp, generatedTimestamp;
    process_state_t oldState, newState;
    state_transition_t transition;
    Process* evtProc;
    bool isValid;

public:
    Event(int ts, int gs, Process* proc, process_state_t os, process_state_t ns, state_transition_t trans);
    int getEvtTimestamp();
    int getGenTimestamp();
    bool isEvtValid();
    void invalidate();
    Process* getEvtProc();
    process_state_t getOldState();
    process_state_t getNewState();
    state_transition_t getTransition();
};

#endif