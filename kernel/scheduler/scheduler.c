#include "scheduler/scheduler.h"
#include <bitset.h>

bool scheduler_ready = false;

proc_desc_t* curr_proc;

// processes that are running, sorted by priority
ll_part_t run_queues[PROC_PRIO_CNT];
// for each priority, is there a process running ?
void* running_bitset;

void init_scheduler()
{
    for (uint32_t prio = 0; prio < PROC_PRIO_CNT; prio++) {
        ll_init(run_queues + prio);    
    }
    running_bitset = bitset_create(PROC_PRIO_CNT);
    create_init_proc(&curr_proc);
    sched_init_proc(curr_proc);
    scheduler_ready = true;
}

uint32_t curr_prio()
{
    return bitset_find_one(running_bitset, PROC_PRIO_CNT);
}

void scheduler_tick(float delta_time)
{
    if (!scheduler_ready) {
        return;
    }
    if (curr_proc->time_left > 0) {
        curr_proc->time_left -= delta_time;
    }
    if (curr_proc->time_left <= 0) {
        curr_proc->need_resched = true;
    }
}

void schedule()
{
    uint32_t prio = curr_prio();
    proc_desc_t* next = run_queues[prio].next;
    ll_rem(&(next->run_queue));
    ll_add_before(&(next->run_queue), run_queues + prio);
    switch_proc(curr_proc, next);
}

void switch_proc(proc_desc_t* prev, proc_desc_t* next)
{
    // initialize the fields we need for the next 
    // running process
    if (next->time_left <= 0) {
        next->time_left = DEFAULT_PROC_TIME;
    }
    next->need_resched = false;

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

void sched_init_proc(proc_desc_t* p)
{
    p->status = STATUS_RUN;
    p->time_left = DEFAULT_PROC_TIME;
    ll_add(&(p->run_queue), run_queues + p->priority);
    bitset_set(running_bitset, p->priority);
}

// called when the thread/process has just been created
void sched_new_proc(proc_desc_t* p, proc_desc_t* creator)
{ 
    // add p to the run queue
    if (p->priority < curr_prio()) {
        curr_proc->need_resched = true;
    }
    printf("A\n");
    p->status = STATUS_RUN;
    ll_add(&(p->run_queue), run_queues + p->priority);
    bitset_set(running_bitset, p->priority);
    printf("B\n");
    // update time left
    creator->time_left *= 0.5;
    p->time_left = creator->time_left;
    printf("C\n");
}

pid_t get_pid()
{
    return curr_proc->pid;
}