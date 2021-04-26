#include "threads/scheduler.h"
#include <linkedlist.h>

// the only process in THREAD_RUNNING state
thread_t* running;
// all processes in THREAD_READY state
ll_part_t ready_list;


void init_scheduler()
{
    ll_init(&ready_list);
}

// called by init_threads
// thread : data of the first thread
void sinit_threads(thread_t* thread)
{
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
    return ll_entry(&(ready_list.next), thread_t, sched_list);
}


void sthread_create(thread_t* thread)
{
    thread->state = THREAD_READY;
    ll_add_before(&(thread->sched_list), &ready_list);
}

void sthread_switch(thread_t* prev, thread_t* next)
{
    // prev was in RUNNING state
    // and will enter READY state
    prev->state = THREAD_READY;
    ll_add_before(&(prev->sched_list), &ready_list);

    // next was in READY state
    // and will enter RUNNING state
    ll_rem(&(next->sched_list));
    next->state = THREAD_RUNNING;
    running = next;
}