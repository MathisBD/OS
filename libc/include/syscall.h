#pragma once
#include <stdint.h>

typedef uint32_t pid_t;


// kernel/user thread
// kernel threads run only in kernel mode 
// and don't use any lower half memory
#define NEW_THREAD_FLAGS_KERNEL   1

// creates a new thread from the current process
// returns the pid of the child
pid_t new_thread(
    // the function to execute
    int (*fn)(void*),
    // the argument to the function
    // it is into eax when the thread starts
    void* arg,
    // the user stack. if creating a kernel
    // thread, not meaningful
    void* stack,
    uint32_t flags
);

// creates a new process from the current process
// no args
// returns the child pid to the parent
// returns 0 to the child
pid_t new_process();


// exits from the current process 
void exit(int code);
