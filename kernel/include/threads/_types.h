#pragma once 
#include <stdint.h>
#include <list.h>

// We need this file because these structs are 
// mutually referential (and the depency graph has a cycle).

struct _thread;
struct _process;
struct _lock;


typedef uint32_t lock_id_t;
typedef struct _lock {
    lock_id_t id;
    list_t* waiting; // list of threads
    uint32_t state;
    struct _thread* owner;
} lock_t;


typedef uint32_t pid_t;
typedef uint32_t lock_id_t;
typedef struct _process {
    pid_t pid;
    list_t* threads; // the threads in this process
    struct _process* parent; // parent process
    list_t* children; // child processes
    uint32_t* page_table;
    // process lifecycle
    uint32_t state;
    int exit_code;
    // process resources (locks, file descriptors are local to a process)
    list_t* locks;
    lock_id_t next_lock_id;
} process_t;


typedef uint32_t tid_t;
typedef struct _thread {
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
    // process this thread is part of.
    // for kernel threads, the parent process is null.
    struct _process* process; 
} thread_t;
