#pragma once
#include <stdint.h>

// thread identifier, local to a process.
typedef uint32_t tid_t;


// creates a thread in the same process.
tid_t thread_create(void(*func)(int), int arg);
void thread_yield();
// the thread never returns from this call
// (it stops running)
void thread_exit(int exit_code);
// returns the exit code of the thread.
// can only join a thread from the same process.
int thread_join(tid_t tid);

