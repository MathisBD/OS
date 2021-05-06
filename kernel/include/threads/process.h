#pragma once
#include <stdint.h>
#include <list.h>
#include "threads/_types.h"
#include "interrupts/interrupts.h"

#define PAGE_TABLE_SIZE 1024

#define PROC_ALIVE  1
// when a process has called proc_exit(),
// it becomes dead : its structures stay allocated,
// until its parent calls proc_wait() on it 
// (at which point it is freed and completely deleted).
#define PROC_DEAD   2


void init_process();

// interrupts should be disabled when calling any of those lock functions.

lock_t* proc_get_lock(process_t* proc, lock_id_t lock_id);
// overwrites the lock->id field with a new id.
// returns this new id.
lock_id_t proc_add_lock(process_t* proc, lock_t* lock);
// doesn't free the lock, only removes it from the process list.
// returns a reference to the removed lock (or zero if the lock wasn't found).
lock_t* proc_remove_lock(process_t* proc, lock_id_t lock_id);


// all these functions are meant to be called
// through system calls.

// forks the current process.
// takes no argument.
// the new process has only one thread,
// that continues executing at the same place as the
// one that created it (i.e. the one calling fork()).
// forking a multithreaded program can be a real pain :
// what if some threads have busy locks ?
// see http://www.linuxprogrammingblog.com/threads-and-fork-think-twice-before-using-them
// for a discussion. I chose to not duplicate the locks (the forked process
// can't access any lock anymore, it has to create new ones). Forking a multithreaded
// process only really makes sense if you're calling exec() afterwards.
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