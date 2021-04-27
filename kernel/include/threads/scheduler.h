#pragma once 
#include "threads/thread.h"
#include <stdbool.h>

// what state to put prev in when performing
// a thread switch ?
#define SWITCH_READY    1
#define SWITCH_FINISH   2
#define SWITCH_WAIT     3


// returns the currently running thread
thread_t* curr_thread();
// returns the thread to run right after the current thread
// i.e. the fist READY thread
// returns 0 if there is no READY thread
thread_t* next_thread();

// these scheduler functions are called by functions
// inside thread.h to update scheduler state

void sinit_threads(thread_t* thread);
// called once the thread has finished been created 
void sthread_create(thread_t* thread); 
// called right BEFORE switching stacks from prev to next
// interrupts should be disabled when calling this function.
// if finish_prev is true, prev will never run again and go 
// to FINISH state.
void sthread_switch(thread_t* prev, thread_t* next, uint32_t switch_mode);

