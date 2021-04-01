#include "scheduler/process.h"
#include "memory/kheap.h"
#include "memory/constants.h"
#include <string.h>


// pid 0 is reserved for the init process
uint32_t curr_pid = 1;

// hashtable : pid -> proc descriptor


pid_t new_pid()
{
    pid_t tmp = curr_pid;
    curr_pid++;
    return tmp;
}


void free_proc(proc_desc_t* proc)
{
    kfree(proc->kstack);
    kfree(proc);
}

void create_init_proc(proc_desc_t** proc)
{
    extern uint32_t init_stack_bottom; // see boot.S

    *proc = kmalloc(sizeof(proc_desc_t));
    (*proc)->kstack = &init_stack_bottom;
    (*proc)->ctx.esp0 = (*proc)->kstack + KSTACK_SIZE;
    
    (*proc)->pid = 0;
}
