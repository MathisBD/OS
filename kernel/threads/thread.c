#include "threads/thread.h"
#include "threads/scheduler.h"
#include <list.h>
#include "memory/kheap.h"
#include "memory/constants.h"
#include "memory/paging.h"
#include "tables/gdt.h"
#include "interrupts/interrupts.h"
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <panic.h>
#include <user_thread.h>
#include "init/init.h"

// code inspired by Operating Systems : Principles and Practice, 
// chapter 4

#define MAX_THREAD_COUNT 1000

// protects access to global thread data, such as next_pid or thread_array.
static queuelock_t* threads_lock;

static thread_t** thread_array;

// TODO : reuse free tids ? with a bitmap or something ?
static tid_t next_tid = 0;
static tid_t _new_tid()
{
    //queuelock_acquire(threads_lock);
    if (next_tid >= MAX_THREAD_COUNT) {
        panic("max thread count reached\n");
    }
    tid_t tmp = next_tid;
    next_tid++;
    //queuelock_release(threads_lock);
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
    thread->lock = queuelock_create();
    thread->join_list = list_create();
    thread->next_waiting = 0;
    extern void init_stack_bottom();
    thread->stack = (uint32_t)init_stack_bottom;
    
    // do the scheduling init stuff
    sinit_threads(thread);
}



tid_t do_thread_create(void (*func)(int), int arg)
{
    bool old_if = set_interrupt_flag(false);
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->tid = new_tid();
    thread->stack = kmalloc(KSTACK_SIZE);
    thread->esp = ((uint32_t)thread->stack) + KSTACK_SIZE;
    thread->join_list = list_create();
    thread->next_waiting = 0;
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
    thread->esp--; *(thread->esp) = new_thread_stub;
    // dummy registers ebx and ebp
    thread->esp--;
    thread->esp--;

    thread_t* curr = curr_thread();
    thread->process = curr->process;
    list_add_back(thread->process->threads, thread);

    sthread_create(thread);
    thread_array[thread->tid] = thread;
    
    set_interrupt_flag(old_if);

    return thread->tid;
}

void do_thread_yield()
{
    sched_switch(SWITCH_READY);
}

void do_thread_exit(int exit_code)
{
    set_interrupt_flag(false);
    thread_t* curr = curr_thread();
    curr->exit_code = exit_code;

    // wake up all threads waiting on the current thread
    while (!list_empty(curr->join_list)) {
        thread_t* thread = list_pop_front(curr->join_list);
        sched_wake_up(thread);
    }

    // code past this will never be executed
    sched_switch(SWITCH_FINISH);
    panic("executing code in finished thread !");    
}

static void delete_thread(thread_t* thread)
{
    thread_array[thread->tid] = 0;
    kfree(thread->stack);
    // the join list should be empty
    list_delete(thread->join_list);
    kfree(thread);
}

int do_thread_join(tid_t tid)
{
    bool old_if = set_interrupt_flag(false);
    thread_t* thread = get_thread(tid);

    // wait for the thread to be finished
    if (thread->state != THREAD_FINISHED) {
        thread_t* curr = curr_thread();
        list_add_back(thread->join_list, curr);
        sched_switch(SWITCH_WAIT);
    }

    int code = thread->exit_code;
    delete_thread(thread);
    set_interrupt_flag(old_if);
    return code;
}