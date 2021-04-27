#pragma once
#include <stdint.h>
#include <list.h>

// implements KERNEL threads

// kthread identifier
typedef uint32_t tid_t;

#define THREAD_RUNNING  2
#define THREAD_READY    3
#define THREAD_WAITING  4
#define THREAD_FINISHED 5


typedef struct {
    tid_t tid;
    //// context info
    uint32_t* stack;
    uint32_t* esp;
    //// scheduling info
    uint32_t state; // run status
    // the list of threads waiting for this thread to finish 
    list_t* join_list;
    //// thread data
    int exit_code;
} thread_t;


void init_threads();
void timer_tick(float seconds);

tid_t thread_create(void(*func)(int), int arg);
void thread_yield();
// the thread never returns from this call
// (it stops running)
void thread_exit(int exit_code);
// returns the exit code of the thread
int thread_join(tid_t tid);

