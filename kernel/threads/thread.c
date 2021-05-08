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
tid_t new_tid()
{
    kql_acquire(threads_lock);

    if (next_tid >= MAX_THREAD_COUNT) {
        panic("max thread count reached\n");
    }
    tid_t tmp = next_tid;
    next_tid++;
    kql_release(threads_lock);
    return tmp;
}

static thread_t* get_thread(tid_t tid)
{
    kql_acquire(threads_lock);
    thread_t* thread = thread_array[tid];
    kql_release(threads_lock);
    return thread;
}

void init_threads()
{
    // create the threads lock
    threads_lock = kql_create();

    thread_array = kmalloc(MAX_THREAD_COUNT * sizeof(thread_t*));
    memset(thread_array, 0, MAX_THREAD_COUNT * sizeof(thread_t*));

    // create a thread context for the kernel
    thread_t* thread = kmalloc(sizeof(thread_t));
    // we can't use next_tid() here since it would acquire a queuelock,
    // which requires the scheduler to be initialized
    thread->tid = next_tid;
    next_tid++;
    thread->lock = kql_create();
    thread->on_finish = event_create(thread->lock);
 
    extern void init_stack_bottom();
    thread->stack = (uint32_t)init_stack_bottom;

    // do the scheduling init stuff
    sinit_threads(thread);
}



tid_t do_thread_create(void (*func)(int), int arg)
{
    queuelock_acquire(threads_lock);

    // no need to lock this thread, we are the only one to have 
    // a pointer to it until this method ends.
    thread_t* thread = kmalloc(sizeof(thread_t));
    thread->tid = new_tid();
    thread->lock = kql_create();
    thread->on_finish = event_create(thread->lock);

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
    thread->esp--; *(thread->esp) = new_thread_stub;
    // dummy registers ebx and ebp
    thread->esp--;
    thread->esp--;

    thread_t* curr = curr_thread();
    thread->process = curr->process;
    
    kql_acquire(thread->process->lock);
    list_add_back(thread->process->threads, thread);
    kql_release(thread->process->lock);

    sthread_create(thread);
    thread_array[thread->tid] = thread;
    
    kql_release(threads_lock);
    return thread->tid;
}

void do_thread_yield()
{
    sched_switch(SWITCH_READY);
}

void do_thread_exit(int exit_code)
{
    thread_t* curr = curr_thread();
    kql_acquire(curr->lock);
    curr->exit_code = exit_code;

    // wake up all threads waiting on the current thread
    event_broadcast(curr->on_finish);
    kql_release(curr->lock);
    
    // code past this will never be executed
    sched_switch(SWITCH_FINISH);
    panic("executing code in finished thread !");    
}

static void delete_thread(thread_t* thread)
{
    kql_acquire(threads_lock);

    kql_acquire(thread->lock);
    thread_array[thread->tid] = 0;
    kfree(thread->stack);
    event_delete(thread->on_finish);
    kql_delete(thread->lock);
    kfree(thread);
    
    kql_release(threads_lock);
}

int do_thread_join(tid_t tid)
{
    thread_t* thread = get_thread(tid);
    kql_acquire(thread->lock);

    // wait for the thread to be finished
    while (thread->state != THREAD_FINISHED) {
        event_wait(thread->on_finish);
    }

    int code = thread->exit_code;
    delete_thread(thread);
    // no need to release the thread->lock, we just deleted it .
    return code;
}