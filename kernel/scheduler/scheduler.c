#include "scheduler/scheduler.h"


proc_desc_t* curr_proc;
// processes that are running
ll_part_t run_head;

void init_scheduler()
{
    ll_init(&run_head);    
    create_init_proc(&curr_proc);
    sched_init_proc(curr_proc);
}


void schedule()
{
    printf("SCHEDULE:curr_proc=%u\n", curr_proc->pid);
    ll_for_each_entry(proc, &run_head, proc_desc_t, run_queue) {
        if (proc != curr_proc) {
            printf("switching to pid=%u\n", proc->pid);
            switch_proc(curr_proc, proc);
        }
    }

}

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
    printf("resumed switch_proc\n");
}

void sched_init_proc(proc_desc_t* proc) 
{
    ll_add(&(proc->run_queue), &run_head);
}

// called when the thread has just been created
// by syscall 'new_thread'
void sched_new_thread(proc_desc_t* thread)
{
    ll_add(&(thread->run_queue), &run_head);
}

pid_t get_pid()
{
    return curr_proc->pid;
}