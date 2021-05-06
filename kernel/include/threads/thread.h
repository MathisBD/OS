#pragma once
#include <stdint.h>
#include "threads/_types.h"

// implements both kernel and user threads.
// user threads are always in a process, whereas
// kernel threads are never in a process.

#define THREAD_RUNNING  2
#define THREAD_READY    3
#define THREAD_WAITING  4
#define THREAD_FINISHED 5



void init_threads();
void timer_tick(float seconds);
tid_t new_tid();
// switch out the current thread for the next READY thread.
// switch_mode indicates what state the current thread should be put in after the switch.
// interrupts should be disabled when calling this function.
void thread_switch(uint32_t switch_mode);


// creates a thread in the same process.
tid_t do_thread_create(void(*func)(int), int arg);
void do_thread_yield();
// the thread never returns from this call
// (it stops running)
void do_thread_exit(int exit_code);
// returns the exit code of the thread.
// can only join a thread from the same process.
int do_thread_join(tid_t tid);

