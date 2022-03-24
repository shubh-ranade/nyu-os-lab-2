#include "process.hpp"

Process::Process(int pid, int at, int tc, int cb, int io, process_state_t s, int st_prio)
    : pid(pid)
    , at(at)
    , tc(tc)
    , cb(cb)
    , io(io)
    , state(s)
    , static_prio(st_prio)
    , dynamic_prio(st_prio - 1)
    , state_ts(at)
    , rem_time(tc)
    , rem_cb(0)
    , iot(0)
    , cwt(0)
{}

int Process::getPID() { return pid; }
int Process::getAT() { return at; }
int Process::getTC() { return tc; }
int Process::getCB() { return cb; }
int Process::getIO() { return io; }
int Process::getStateTS() { return state_ts; }
int Process::getRemTime() { return rem_time; }
int Process::getRemCB() { return rem_cb; }
int Process::getFT() { return ft; }
int Process::getTT() { return tt; }
int Process::getIOT() { return iot; }
int Process::getCWT() { return cwt; }
int Process::getStPrio() { return static_prio; }
int Process::getPrio() { return dynamic_prio; }
void Process::setState(process_state_t s) { state = s; }
void Process::setStateTS(int ts) { state_ts = ts; }
void Process::setRemTime(int rt) { rem_time = rt; }
void Process::setRemCB(int t) { rem_cb = t; }
void Process::setFT(int t) { ft = t; }
void Process::setTT(int t) { tt = t; }
void Process::setIOT(int t) { iot = t; }
void Process::setCWT(int t) { cwt = t; }
void Process::setStaticPrio(int p) { static_prio = p; }
void Process::setDynamicPrio(int p) { dynamic_prio = p; }
process_state_t Process::getState() { return state; }
