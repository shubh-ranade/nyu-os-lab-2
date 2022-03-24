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
    Process(int pid, int at, int tc, int cb, int io, process_state_t s, int st_prio);
    int getPID();
    int getAT();
    int getTC();
    int getCB();
    int getIO();
    int getStateTS();
    int getRemTime();
    int getRemCB();
    int getFT();
    int getTT();
    int getIOT();
    int getCWT();
    int getStPrio();
    int getPrio();
    void setState(process_state_t s);
    void setStateTS(int ts);
    void setRemTime(int rt);
    void setRemCB(int t);
    void setFT(int t);
    void setTT(int t);
    void setIOT(int t);
    void setCWT(int t);
    void setStaticPrio(int p);
    void setDynamicPrio(int p);
    process_state_t getState();
};

#endif
