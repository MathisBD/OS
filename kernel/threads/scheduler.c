#include "threads/scheduler.h"
#include <list.h>
#include <stdbool.h>
#include <stdio.h>
#include <panic.h>

// the only process in THREAD_RUNNING state
thread_t* running;
// all processes in THREAD_READY state
list_t* ready_list;
// all processes in THREAD_FINISHED state
list_t* finished_list;

// called by init_threads
// thread : data of the first thread
void sinit_threads(thread_t* thread)
{
    ready_list = list_create();
    finished_list = list_create();

    thread->state = THREAD_RUNNING;
    running = thread;    
}

thread_t* curr_thread()
{
    return running;
}

thread_t* next_thread()
{
    if (list_empty(ready_list)) {
        return 0;
    }
    return ready_list->first->contents;
}

void sthread_create(thread_t* thread)
{
    thread->state = THREAD_READY;
    list_add_back(ready_list, (void*)thread);
}

void sthread_switch(uint32_t switch_mode)
{
    thread_t* prev = running;
    thread_t* next = list_pop_front(ready_list);
    // prev was in RUNNING state
    if (switch_mode == SWITCH_FINISH) {
        prev->state = THREAD_FINISHED;
        // add to the finished list
        list_add_back(finished_list, (void*)prev);
    }
    else if (switch_mode == SWITCH_READY) {
        prev->state = THREAD_READY;
        // add to the end of the ready list
        list_add_back(ready_list, (void*)prev);
    }
    else if (switch_mode == SWITCH_WAIT) {
        prev->state = THREAD_WAITING;
    }
    else {
        panic("unknown switch mode\n");
    }

    // next was in READY state
    // and will enter RUNNING state
    next->state = THREAD_RUNNING;
    running = next;
}

void swake_up(thread_t* thread)
{
    if (thread->state != THREAD_WAITING) {
        panic("can't wake up a thread that isn't waiting");
    }
    thread->state = THREAD_READY;
    list_add_back(ready_list, thread);
}
