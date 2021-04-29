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
// from user space only (through system calls).

// forks the current process.
// the new process has only one thread,
// that continues executing at the same place as the
// one that created it (i.e. the one calling fork()).
void do_proc_fork(intr_frame_t* frame);
// replaces the user code/data for the current process.
// only works if the process has a single thread.
void do_proc_exec(intr_frame_t* frame);
// when a thread calls exit, all threads in the process are terminated.
void do_proc_exit(intr_frame_t* frame);
// wait on a process. blocks the thread that calls it.
int do_proc_wait(intr_frame_t* frame);