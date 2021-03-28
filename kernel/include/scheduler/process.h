#pragma once
#include <linkedlist.h>
#include <stdint.h>

// each process has its own kernel mode stack
// in addition to its own user mode stack
#define PROC_KSTACK_SIZE 8096

typedef uint32_t pid_t;

typedef struct {
    void* kstack;
    // registers
} proc_ctx_t;

typedef struct {
    pid_t pid;
    // hardware context
    proc_ctx_t ctx;
    // scheduling info
    uint32_t status;
    ll_part_t run_queue;
    // relationship info
    ll_part_t children_head;
    ll_part_t siblings;
} proc_desc_t;

void find_proc(pid_t pid);
void create_proc(proc_desc_t** proc, pid_t* pid);
void del_proc(proc_desc_t* proc);