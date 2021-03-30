#include "scheduler/scheduler.h"


proc_desc_t* curr_proc;
// processes that are running
ll_part_t run_head;


void switch_proc(proc_desc_t* prev, proc_desc_t* next)
{
    extern void switch_asm(
        proc_desc_t* prev, 
        proc_desc_t* next, 
        proc_desc_t* last, 
        uint32_t ctx_ofs);

    proc_desc_t* last;
    switch_asm(prev, next, last, offsetof(proc_desc_t, ctx));

    // when execution of process prev resumes,
    // the code here will be executed

}

pid_t get_pid()
{
    return curr_proc->pid;
}