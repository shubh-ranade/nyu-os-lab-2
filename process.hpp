#ifndef PROCESS_HPP
#define PROCESS_HPP

// Process states
typedef enum { CREATED, READY, RUNNING, BLOCKED } process_state_t;

// State transitions
typedef enum {
    CREATED_TO_READY, READY_TO_RUN, RUN_TO_BLOCKED, BLOCKED_TO_READY, RUN_TO_READY
} state_transition_t;

class Process {
private:
    int pid, at, tc, cb, io;
    // state_ts: ts when the process was put in current state "state"
    int state_ts, rem_time, rem_cb;
    int ft, tt, iot, cwt;
    int static_prio, dynamic_prio;
    process_state_t state;

public:
    Process(int pid, int at, int tc, int cb, int io, process_state_t s, int st_prio)
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

    int getPID() { return pid; }
    int getAT() { return at; }
    int getTC() { return tc; }
    int getCB() { return cb; }
    int getIO() { return io; }
    int getStateTS() { return state_ts; }
    int getRemTime() { return rem_time; }
    int getRemCB() { return rem_cb; }
    int getFT() { return ft; }
    int getTT() { return tt; }
    int getIOT() { return iot; }
    int getCWT() { return cwt; }
    int getStPrio() { return static_prio; }
    int getPrio() { return dynamic_prio; }
    void setState(process_state_t s) { state = s; }
    void setStateTS(int ts) { state_ts = ts; }
    void setRemTime(int rt) { rem_time = rt; }
    void setRemCB(int t) { rem_cb = t; }
    void setFT(int t) { ft = t; }
    void setTT(int t) { tt = t; }
    void setIOT(int t) { iot = t; }
    void setCWT(int t) { cwt = t; }
    void setStaticPrio(int p) { static_prio = p; }
    void setDynamicPrio(int p) { dynamic_prio = p; }
    process_state_t getState() { return state; }
};

#endif
