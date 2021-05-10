#pragma once
#include <stdint.h>
#include "threads/process.h"
#include "sync/queuelock.h"
#include "sync/event.h"

// implements both kernel and user threads.
// user threads are always in a process, whereas
// kernel threads are never in a process.

#define THREAD_RUNNING  2
#define THREAD_READY    3
#define THREAD_WAITING  4
#define THREAD_FINISHED 5

typedef uint32_t tid_t;
typedef struct _thread {
    tid_t tid;
    queuelock_t* lock;
    //// context info
    uint32_t* stack;
    uint32_t* esp;
    //// scheduling info
    uint32_t state; // run status
    // next thread in the ready list
    struct _thread* sched_next;
    // join() waits on this event.
    // exit() broadcasts this event.
    event_t* on_finish;
    //// thread data
    int exit_code;
    // process this thread is part of.
    process_t* process; 
} thread_t;



void init_threads();
tid_t new_tid();

// creates a thread in the same process.
tid_t kthread_create(void(*func)(int), int arg);
void kthread_yield();
// the thread never returns from this call
// (it stops running)
void kthread_exit(int exit_code);
// returns the exit code of the thread.
// can only join a thread from the same process.
int kthread_join(tid_t tid);

