#include "threads/thread.h"
#include "threads/scheduler.h"
#include <linkedlist.h>
#include "memory/kheap.h"
#include "memory/constants.h"
#include "interrupts/interrupts.h"
#include <stdio.h>
#include <stdbool.h>

// code inspired by Operating Systems : Principles and Practice, 
// chapter 4

// TODO : reuse free tids ? with a bitmap or something ?
tid_t next_tid = 0;
tid_t new_tid()
{
    tid_t tmp = next_tid;
    next_tid++;
    return tmp;
}

void init_threads()
{
    // create a thread context for the kernel
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->tid = new_tid();
    thread->stack = kmalloc(KSTACK_SIZE);
    thread->esp = thread->stack + KSTACK_SIZE;

    // do the scheduling init stuff
    sinit_threads(thread);
}

// interrupts should be disabled when calling this function
extern void thread_switch_asm(
    thread_t* prev, 
    thread_t* next,
    uint32_t esp_ofs);
void thread_switch(thread_t* prev, thread_t* next, bool finish_prev)
{
    sthread_switch(prev, next, finish_prev);
    uint32_t esp_ofs = offsetof(thread_t, esp);
    thread_switch_asm(prev, next, esp_ofs);
}

// first function a newly created thread executes
void stub(void (*func)(int), int arg)
{
    (*func)(arg);
    thread_exit(0); // in case the function didn't call exit already
}

tid_t thread_create(void (*func)(int), int arg)
{
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->tid = new_tid();
    thread->stack = kmalloc(KSTACK_SIZE);
    thread->esp = ((uint32_t)thread->stack) + KSTACK_SIZE;
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
    return thread->tid;
}

void thread_yield()
{
    disable_interrupts();
    thread_t* next = next_thread();
    // check there is actually another thread to run
    if (next != 0) {
        thread_t* curr = curr_thread();
        thread_switch(curr, next, false);
    }
    enable_interrupts();
}

void thread_exit(int exit_code)
{
    disable_interrupts();

    thread_t* curr = curr_thread();
    curr->exit_code = exit_code;

    // switch threads
    thread_t* next = next_thread();
    // check there is actually another thread to run
    if (next != 0) {
        thread_switch(curr, next, true);
    }
    else {
        panic("can't exit from the only thread alive\n");
    }
    enable_interrupts();
}
