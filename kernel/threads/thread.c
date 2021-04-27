#include "threads/thread.h"
#include "threads/scheduler.h"
#include <list.h>
#include "memory/kheap.h"
#include "memory/constants.h"
#include "interrupts/interrupts.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

// code inspired by Operating Systems : Principles and Practice, 
// chapter 4

#define MAX_THREAD_COUNT 1000

thread_t** thread_array;
bool enabled_threads = false;

// TODO : reuse free tids ? with a bitmap or something ?
tid_t next_tid = 0;
tid_t new_tid()
{
    if (next_tid >= MAX_THREAD_COUNT) {
        panic("max thread count reached\n");
    }
    tid_t tmp = next_tid;
    next_tid++;
    return tmp;
}

thread_t* get_thread(tid_t tid)
{
    return thread_array[tid];
}

void init_threads()
{
    thread_array = kmalloc(MAX_THREAD_COUNT * sizeof(thread_t*));
    memset(thread_array, 0, MAX_THREAD_COUNT * sizeof(thread_t*));

    // create a thread context for the kernel
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->tid = new_tid();
    thread->stack = kmalloc(KSTACK_SIZE);
    thread->esp = thread->stack + KSTACK_SIZE;

    // do the scheduling init stuff
    sinit_threads(thread);

    // set the flag
    enabled_threads = true;
}

// switch out the current thread for the next READY thread.
// switch_mode indicates what state the current thread should be put in after the switch
// interrupts should be disabled when calling this function
extern void thread_switch_asm(
    thread_t* prev, 
    thread_t* next,
    uint32_t esp_ofs);
void thread_switch(uint32_t switch_mode)
{
    thread_t* prev = curr_thread();
    thread_t* next = next_thread();
    if (next == 0) {
        if (switch_mode == SWITCH_READY) {
            return;
        }
        panic("no more READY thread\n");
    }
    sthread_switch(switch_mode);
    uint32_t esp_ofs = offsetof(thread_t, esp);
    thread_switch_asm(prev, next, esp_ofs);
}

void timer_tick(float seconds)
{
    disable_interrupts();
    if (enabled_threads) {
        thread_switch(SWITCH_READY);
    }
    enable_interrupts();
}

// first function a newly created thread executes
void stub(void (*func)(int), int arg)
{
    // interrupts were disabled before switching threads.
    enable_interrupts();
    (*func)(arg);
    thread_exit(0); // in case the function didn't call exit already
}

tid_t thread_create(void (*func)(int), int arg)
{
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->tid = new_tid();
    thread->stack = kmalloc(KSTACK_SIZE);
    thread->esp = ((uint32_t)thread->stack) + KSTACK_SIZE;
    thread->join_list = list_create();
    // setup a dummy stack frame for the new thread
    // so that it can be started as if resuming from a switch.
    // this frame has to mimmick that of a thread that was switched out
    // in thread_switch.
    // stub arguments
    thread->esp--; *(thread->esp) = arg;
    thread->esp--; *(thread->esp) = func;
    // stub dummy return adress
    // this is needed so that stub can correctly find
    // its arguments on the stack
    thread->esp--;
    // thread_switch return address
    // (no need to push dummy arguments for thread_switch)
    thread->esp--; *(thread->esp) = stub;
    // dummy registers ebx and ebp
    thread->esp--;
    thread->esp--;

    sthread_create(thread);

    thread_array[thread->tid] = thread;
    return thread->tid;
}

void thread_yield()
{
    disable_interrupts();
    thread_switch(SWITCH_READY);
    enable_interrupts();
}

void thread_exit(int exit_code)
{
    disable_interrupts();
    thread_t* curr = curr_thread();
    curr->exit_code = exit_code;

    // wake up all threads waiting on the current thread
    while (!list_empty(curr->join_list)) {
        thread_t* thread = list_pop_front(curr->join_list);
        swake_up(thread);
    }

    // code past this will never be executed
    thread_switch(SWITCH_FINISH);
    panic("executing code in finished thread !");    
}

void delete_thread(thread_t* thread)
{
    thread_array[thread->tid] = 0;
    kfree(thread->stack);
    // the join list should be empty
    list_delete(thread->join_list);
    kfree(thread);
}

int thread_join(tid_t tid)
{
    disable_interrupts();
    thread_t* thread = get_thread(tid);

    // wait for the thread to be finished
    if (thread->state != THREAD_FINISHED) {
        thread_t* curr = curr_thread();
        list_add_back(thread->join_list, curr);
        thread_switch(SWITCH_WAIT);
    }

    int code = thread->exit_code;
    delete_thread(thread);
    enable_interrupts();
    return code;
}