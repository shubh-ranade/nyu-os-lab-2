#include "event.hpp"

Event::Event(int ts, int gs, Process* proc, process_state_t os, process_state_t ns, state_transition_t trans)
        : evtTimestamp(ts)
        , generatedTimestamp(gs)
        , evtProc(proc)
        , oldState(os)
        , newState(ns)
        , transition(trans)
        , isValid(true)
    {}

int Event::getEvtTimestamp() { return evtTimestamp; }
int Event::getGenTimestamp() { return generatedTimestamp; }
bool Event::isEvtValid() { return isValid; }
void Event::invalidate() { isValid = false; }
Process* Event::getEvtProc() { return evtProc; }
process_state_t Event::getOldState() { return oldState; }
process_state_t Event::getNewState() { return newState; }
state_transition_t Event::getTransition() { return transition; }
