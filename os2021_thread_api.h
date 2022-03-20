#ifndef OS2021_API_H
#define OS2021_API_H

#define STACK_SIZE 8192

#include <stdio.h>
#include <stdlib.h>
#include <ucontext.h>
#include <sys/time.h>
#include <signal.h>
#include <unistd.h>
#include <stdbool.h>
#include <string.h>
#include "function_libary.h"


struct thread
{
    char *th_name;
    int thread_tid;
    //priority: H=2, M=1, L=0
    int b_priority;//original priority
    int c_priority;//current priority
    int thread_cancel_mode;//0=asynchronous, 1=deffer(notice: reclaimer can't goto the end)
    int thread_cancel_status;//0=nothing, 1=waiting cancel
    int thread_wait;//event I want wait(0~7), -1 = no waiting
    long thread_qtime;//queueing time
    long thread_wtime;//waiting time
    long thread_waittime;//save how long you need to wait
    long thread_already_wait;//use when ThreadWaitTime
    ucontext_t thread_ctx;//context recored
    struct thread *thread_next;
};
typedef struct thread thread;
typedef struct thread * thread_pointer;
/*multilevel feedback queue*/
void enq(thread_pointer *, thread_pointer *);
thread_pointer deq(thread_pointer *);
thread_pointer create_thread(char *, int, char *, int);
void priority_change(thread_pointer *, int, int);
int OS2021_ThreadCreate(char *job_name, char *p_function, char *priority, int cancel_mode);
void OS2021_ThreadCancel(char *job_name);
void OS2021_ThreadWaitEvent(int event_id);
void OS2021_ThreadSetEvent(int event_id);
void OS2021_ThreadWaitTime(int msec);
void OS2021_DeallocateThreadResource();
void OS2021_TestCancel();


void CreateContext(ucontext_t *, ucontext_t *, void *);
void ResetTimer();
void Dispatcher();
void StartSchedulingSimulation();

#endif
