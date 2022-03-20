#ifndef SELF_THREAD_H
#define SELF_THREAD_H

#include <ucontext.h>
#include <stdlib.h>
#include <stdio.h>

/*self thread structure*/
struct thread
{
    char *th_name;
    int th_id;
    //priority: H=2, M=1, L=0
    int b_priority;//original priority
    int th_priority;//current priority
    int th_cancelmode;//0=asynchronous, 1=deffer(notice: reclaimer can't goto the end)
    int th_cancel_status;//0=nothing, 1=waiting cancel
    int th_wait;//event I want wait(0~7), -1 = no waiting
    long th_qtime;//queueing time
    long th_wtime;//waiting time
    long th_waittime;//save how long you need to wait
    long th_already_wait;//use when ThreadWaitTime
    ucontext_t th_ctx;//context recored
    struct thread *th_next;
};
typedef struct thread thread_t;
typedef struct thread * thread_tptr;
/*multilevel feedback queue*/
void enq(thread_tptr *, thread_tptr *);
thread_tptr deq(thread_tptr *);
thread_tptr create_thread(char *, int, char *, int);
void priority_change(thread_tptr *, int, int);

#endif
