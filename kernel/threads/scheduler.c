#include "threads/scheduler.h"
#include <linkedlist.h>
#include <stdbool.h>
#include <stdio.h>

// the only process in THREAD_RUNNING state
thread_t* running;
// all processes in THREAD_READY state
ll_part_t ready_list;
// all processes in THREAD_FINISHED state
ll_part_t finished_list;

// called by init_threads
// thread : data of the first thread
void sinit_threads(thread_t* thread)
{
    ll_init(&ready_list);
    ll_init(&finished_list);
    
    thread->state = THREAD_RUNNING;
    running = thread;    
}

thread_t* curr_thread()
{
    return running;
}

thread_t* next_thread()
{
    if (ll_empty(&ready_list)) {
        return 0;
    }
    return ll_entry(ready_list.next, thread_t, sched_list);
}


void sthread_create(thread_t* thread)
{
    thread->state = THREAD_READY;
    ll_add_before(&(thread->sched_list), &ready_list);
}

void sthread_switch(thread_t* prev, thread_t* next, bool finish_prev)
{
    printf("switching from %u to %u\n", prev->tid, next->tid);
    // prev was in RUNNING state
    if (finish_prev) {
        // prev will enter FINISHED state
        prev->state = THREAD_FINISHED;
        // add wherever we want in the finished list
        ll_add(&(prev->sched_list), &finished_list);
    }
    else {
        // prev will enter READY state
        prev->state = THREAD_READY;
        // add to the end of the ready list
        ll_add_before(&(prev->sched_list), &ready_list);
    }
    // next was in READY state
    // and will enter RUNNING state
    ll_rem(&(next->sched_list));
    next->state = THREAD_RUNNING;
    running = next;
}
