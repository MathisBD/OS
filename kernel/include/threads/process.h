#pragma once
#include <stdint.h>
#include <list.h>
#include "interrupts/interrupts.h"

#define PAGE_TABLE_SIZE 1024

#define PROC_ALIVE  1
// when a process has called proc_exit(),
// it becomes dead : its structures stay allocated,
// until its parent calls proc_wait() on it 
// (at which point it is freed and completely deleted).
#define PROC_DEAD   2

typedef uint32_t pid_t;

typedef struct __process {
    pid_t pid;
    list_t* threads; // the threads in this process
    struct __process* parent; // parent process
    list_t* children; // child processes
    uint32_t* page_table;
    // process lifecycle
    uint32_t state;
    int exit_code;
} process_t;


void init_process();

// all these functions are meant to be called
// through system calls.

// forks the current process.
// the new process has only one thread,
// that continues executing at the same place as the
// one that created it (i.e. the one calling fork()).
// takes no argument.
void do_proc_fork(intr_frame_t* frame);
// replaces the user code/data for the current process.
// only works if the process has a single thread.
// (this ensures no thread is waiting on this thread,
// thus we don't have to call thread_exit() for this thread
// and we can just plainly replace it).
// arguments :
// 1) char* : program to run (full path to the file)
void do_proc_exec(intr_frame_t* frame);
// when a thread calls exit, all threads in the process are terminated.
// arguments :
// 1) int : exit code
void do_proc_exit(intr_frame_t* frame);
// wait on a process. blocks the thread that calls it.
// arguments :
// 1) pid_t : process to wait on. has to be a child of the current process.
int do_proc_wait(intr_frame_t* frame);