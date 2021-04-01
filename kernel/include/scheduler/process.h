#pragma once
#include <linkedlist.h>
#include <stdint.h>
#include <stdbool.h>


typedef uint32_t pid_t;

typedef struct {
    uint32_t esp;   // current esp 
    uint32_t esp0;  // esp to use for the tss (i.e. top of the stack)
    uint32_t eip;   // where to return to when switching back to this process
} proc_ctx_t;


typedef struct {
    pid_t pid;
    // hardware context
    // each process has its own kernel mode stack
    // in addition to its own user mode stack (optional)
    void* kstack;
    proc_ctx_t ctx;
    // scheduling info
    //uint32_t status;
    //ll_part_t run_queue;
    // relationship info
    //ll_part_t children_head;
    //ll_part_t siblings;
} proc_desc_t;

// returns an unused pid
pid_t new_pid();

void find_proc(pid_t pid);
void free_proc(proc_desc_t* proc);
// creates the init process
void create_init_proc(proc_desc_t** proc);

