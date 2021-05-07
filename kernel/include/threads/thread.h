#pragma once
#include <stdint.h>
#include "threads/process.h"
#include "sync/queuelock.h"

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
    // if we are waiting on a lock,
    // the next thread waiting on the same lock
    struct _thread* next_waiting;
    // the list of threads waiting for this thread to finish 
    list_t* join_list;
    //// thread data
    int exit_code;
    // process this thread is part of.
    process_t* process; 
} thread_t;



void init_threads();
tid_t new_tid();

// creates a thread in the same process.
tid_t do_thread_create(void(*func)(int), int arg);
void do_thread_yield();
// the thread never returns from this call
// (it stops running)
void do_thread_exit(int exit_code);
// returns the exit code of the thread.
// can only join a thread from the same process.
int do_thread_join(tid_t tid);

