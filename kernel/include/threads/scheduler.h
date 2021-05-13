#pragma once 
#include "threads/thread.h"
#include <stdbool.h>
#include "sync/spinlock.h"
#include "sync/queuelock.h"


// returns the currently running thread/process
thread_t* curr_thread();
process_t* curr_process();

void sched_wake_up(thread_t* thread);
// switch out the current thread, and put it in READY state
void sched_switch();
// atomically release the lock and switch out the current thread,
// putting it in waiting state.
void sched_suspend_and_release_spinlock(spinlock_t*);
void sched_suspend_and_release_queuelock(queuelock_t*);
// atomically release the lock and switch out the current thread,
// terminating the current thread/process.
void sched_finish_thread_and_release(queuelock_t*);
void sched_finish_proc_and_release(queuelock_t*);

void timer_tick(float seconds);

// first function a new thread (created by thread_create() or 
// proc_fork()) executes.
// should not be called directly (see thread.c and process.c).
void new_thread_stub(void (*func)(int), int arg);
void forked_thread_stub();

// callback functions

// called by init_thread()
void sinit_threads(thread_t* thread);
// called once a thread has finished been created 
void sthread_create(thread_t* thread); 
// called right BEFORE switching stacks from prev to next
void sthread_switch(uint32_t switch_mode);
