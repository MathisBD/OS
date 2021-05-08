#include "threads/scheduler.h"
#include <list.h>
#include <stdbool.h>
#include <stdio.h>
#include <panic.h>
#include "sync/spinlock.h"
#include "interrupts/interrupts.h"
#include "memory/constants.h"
#include "memory/paging.h"
#include "init/init.h"
#include "tables/gdt.h"
#include <user_thread.h>

#define LOCK() \
bool _old_if = set_interrupt_flag(false); \
ksl_acquire(sched_spinlock);

#define UNLOCK() \
ksl_release(sched_spinlock); \
set_interrupt_flag(_old_if); 

// the scheduler lock.
// this has to be a spinlock.
// we must DISABLE INTERRUPTS before acquiring this lock,
// otherwise interrutps might try to acquire it while it is busy,
// causing a deadlock.
static spinlock_t* sched_spinlock;

// the only process in THREAD_RUNNING state
static thread_t* running;
// all processes in THREAD_READY state
static list_t* ready_list;
// all processes in THREAD_FINISHED state
static list_t* finished_list;

// called by init_threads
// thread : data of the first thread
void sinit_threads(thread_t* thread)
{
    ready_list = list_create();
    finished_list = list_create();

    thread->state = THREAD_RUNNING;
    running = thread;   

    sched_spinlock = ksl_create(); 
}

thread_t* curr_thread()
{
    return running;
}

process_t* curr_process()
{
    return running->process;
}


void sthread_create(thread_t* thread)
{
    LOCK();
    thread->state = THREAD_READY;
    list_add_back(ready_list, thread);
    UNLOCK();
}

// first function a thread created with thread_create() executes
void new_thread_stub(void (*func)(int), int arg)
{
    // before switching threads, interrupts were disabled and
    // the scheduler lock was acquired.
    ksl_release(sched_spinlock);
    set_interrupt_flag(true);
    (*func)(arg);
    kthread_exit(0); // in case the function didn't call exit already
}

// first function a thread created with proc_fork() executes
void forked_thread_stub()
{
    // before switching threads, interrupts were disabled and
    // the scheduler lock was acquired.
    ksl_release(sched_spinlock);
    set_interrupt_flag(true);
    // simply pop an address from the stack and jump to it.
    return;
}

extern void thread_switch_asm(
    thread_t* prev, 
    thread_t* next,
    uint32_t esp_ofs,
    uint32_t page_table_address // physical address
);
static void thread_switch(thread_t* prev, thread_t* next)
{
    //printf("switch %x->%x\n", prev->tid, next->tid);
    set_tss_esp(((uint32_t)(next->stack)) + KSTACK_SIZE);
    uint32_t pt_addr = physical_address(next->process->page_table);
    thread_switch_asm(prev, next, offsetof(thread_t, esp), pt_addr);
}

void sched_switch(uint32_t switch_mode)
{
    LOCK();

    thread_t* prev = running;
    if (list_empty(ready_list)) {
        if (switch_mode == SWITCH_READY) {
            UNLOCK();
            return;
        }
        else {
            panic("no ready thread to switch to !");
        }
    }
    thread_t* next = list_pop_front(ready_list);

    // prev was in RUNNING state
    switch (switch_mode) {
    case SWITCH_FINISH:
    {
        prev->state = THREAD_FINISHED;
        // add to the finished list
        list_add_back(finished_list, (void*)prev);
        break;
    }
    case SWITCH_READY:
    {
        prev->state = THREAD_READY;
        // add to the end of the ready list
        list_add_back(ready_list, (void*)prev);
        break;
    }
    case SWITCH_WAIT: 
    {
        prev->state = THREAD_WAITING;
        break;
    }
    default:
    {
        panic("unknown switch mode\n");
    }
    }

    // next was in READY state
    // and will enter RUNNING state
    next->state = THREAD_RUNNING;
    running = next;

    // do the actual switch
    thread_switch(prev, next);
    UNLOCK();
}

void timer_tick(float seconds)
{
    if (is_all_init()) {
        sched_switch(SWITCH_READY);
    }
}


void sched_wake_up(thread_t* thread)
{
    LOCK();
    if (thread->state != THREAD_WAITING) {
        panic("can't wake up a thread that isn't waiting");
    }
    thread->state = THREAD_READY;
    list_add_back(ready_list, thread);
    UNLOCK();
}

void sched_suspend_and_release_spinlock(spinlock_t* lock)
{
    LOCK();
    ksl_release(lock);

    thread_t* prev = running;
    if (list_empty(ready_list)) {
        panic("no ready thread to switch to (sched_suspend_and_release_spinlock) !");
    }
    thread_t* next = list_pop_front(ready_list);
    prev->state = THREAD_WAITING;
    next->state = THREAD_RUNNING;
    running = next;
    thread_switch(prev, next);
    UNLOCK();
}

void sched_suspend_and_release_queuelock(queuelock_t* lock)
{
    LOCK();
    kql_release(lock);

    thread_t* prev = running;
    if (list_empty(ready_list)) {
        panic("no ready thread to switch to (sched_suspend_and_release_queuelock) !");
    }
    thread_t* next = list_pop_front(ready_list);
    prev->state = THREAD_WAITING;
    next->state = THREAD_RUNNING;
    running = next;
    thread_switch(prev, next);
    UNLOCK();
}