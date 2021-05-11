#pragma once
#include <stdint.h>
#include <list.h>
#include <map.h>
#include "interrupts/interrupts.h"
#include "sync/queuelock.h"
#include "sync/event.h"
#include "filesystem/file_descr.h"


#define PAGE_TABLE_SIZE 1024

#define PROC_ALIVE  1
// when a process has called proc_exit(),
// it becomes dead : its structures stay allocated,
// until its parent calls proc_wait() on it 
// (at which point it is freed and completely deleted).
#define PROC_DEAD   2

#define FD_ID_STDOUT    0
#define FD_ID_STDIN     1


typedef uint32_t pid_t;
typedef struct _process {
    pid_t pid;
    queuelock_t* lock;
    list_t* threads; // the threads in this process
    struct _process* parent; // parent process
    list_t* children; // child processes
    uint32_t* page_table;
    // process lifecycle
    uint32_t state;
    int exit_code;
    // process resources (local to the process)
    map_t* file_descrs;
    uint32_t next_fdid;
    map_t* locks;
    uint32_t next_lid;
    map_t* events;
    uint32_t next_eid;
} process_t;

void init_process();

// user locks
uint32_t proc_add_lock(process_t*, queuelock_t*);
queuelock_t* proc_get_lock(process_t*, uint32_t id);
queuelock_t* proc_remove_lock(process_t*, uint32_t id);

// user events
uint32_t proc_add_event(process_t*, event_t*);
event_t* proc_get_event(process_t*, uint32_t id);
event_t* proc_remove_event(process_t*, uint32_t id);

// user file descriptors
uint32_t proc_add_fd(process_t*, file_descr_t*);
file_descr_t* proc_get_fd(process_t*, uint32_t id);
file_descr_t* proc_remove_fd(process_t*, uint32_t id);


// forks the current process.
// takes no argument.
// this has to be called through a SYSTEM CALL.
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
// argument : program to run (full path to the file)
void kproc_exec(char* prog, uint32_t argc, char** argv);
// when a thread calls exit, all threads in the process are terminated.
void kproc_exit(int code);
// wait on a child process. blocks the thread that calls it.
// returns the exit code of the process.
int kproc_wait(pid_t pid);