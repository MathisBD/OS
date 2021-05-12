#include "threads/scheduler.h"
#include <list.h>
#include <stdbool.h>
#include <stdio.h>
#include "sync/spinlock.h"
#include "interrupts/interrupts.h"
#include "memory/constants.h"
#include "memory/paging.h"
#include "init/init.h"
#include "tables/gdt.h"
#include <user_thread.h>
#include "drivers/vga_driver.h"


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
// first/last thread in the ready list.
// we can't use regular lists here, because we 
// can't use the heap (i.e. malloc/free) when switching between
// threads : interrupts are disabled during the timer interrupt,
// and aren't enabled before the thread switch is complete.
// if we use the heap in this interval, we need to acquire the 
// heap spinlock, which might be busy if an interrupted process
// was using it : deadlock.
static thread_t* first_ready;
static thread_t* last_ready;


// called by init_threads
// thread : data of the first thread
void sinit_threads(thread_t* thread)
{
    first_ready = 0;
    last_ready = 0;

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

static void add_ready(thread_t* thread)
{
    if (last_ready == 0) {
        first_ready = thread;
        last_ready = thread;
    }
    else {
        last_ready->sched_next = thread;
        last_ready = thread;
    }
}

static thread_t* pop_ready()
{
    if (first_ready == 0) {
        return 0;
    }
    else if (first_ready == last_ready) {
        thread_t* thread = first_ready;
        first_ready = 0;
        last_ready = 0;
        return thread;
    }
    else {
        thread_t* thread = first_ready;
        first_ready = first_ready->sched_next;
        return thread;
    }
}

static thread_t* next_ready()
{
    return first_ready;
}


void sthread_create(thread_t* thread)
{
    LOCK();
    thread->state = THREAD_READY;
    add_ready(thread);
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
    // we can't use the standard vga methods since interrupts are disabled
    /*vga_print("switch ");
    vga_print_mem(&(prev->tid), 4);
    vga_print("-> ");
    vga_print_mem(&(next->tid), 4);
    vga_print("\n");*/

    set_tss_esp(((uint32_t)(next->stack)) + KSTACK_SIZE);
    uint32_t pt_addr = physical_address(next->process->page_table);
    thread_switch_asm(prev, next, offsetof(thread_t, esp), pt_addr);
}

void sched_switch(uint32_t switch_mode)
{
    LOCK();

    thread_t* prev = running;
    if (next_ready() == 0) {
        if (switch_mode == SWITCH_READY) {
            UNLOCK();
            return;
        }
        else {
            vga_print("no ready thread to switch to !");
            while(1);
        }
    }
    thread_t* next = pop_ready();

    // prev was in RUNNING state
    switch (switch_mode) {
    case SWITCH_FINISH:
    {
        prev->state = THREAD_FINISHED;
        break;
    }
    case SWITCH_READY:
    {
        prev->state = THREAD_READY;
        // add to the end of the ready list
        add_ready(prev);
        break;
    }
    case SWITCH_WAIT: 
    {
        prev->state = THREAD_WAITING;
        break;
    }
    default:
    {
        vga_print("unknown switch mode\n");
        while(1);
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
    sched_switch(SWITCH_READY);
}


void sched_wake_up(thread_t* thread)
{
    LOCK();
    if (thread->state != THREAD_WAITING) {
        vga_print("can't wake up a thread that isn't waiting");
        while(1);
    }
    thread->state = THREAD_READY;
    add_ready(thread);
    UNLOCK();
}

void sched_suspend_and_release_spinlock(spinlock_t* lock)
{
    LOCK();
    ksl_release(lock);

    thread_t* prev = running;
    if (next_ready() == 0) {
        vga_print("no ready thread to switch to (sched_suspend_and_release_spinlock) !");
        while(1);
    }
    thread_t* next = pop_ready();
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
    if (next_ready() == 0) {
        vga_print("no ready thread to switch to (sched_suspend_and_release_queuelock) !");
        while(1);
    }
    thread_t* next = pop_ready();
    prev->state = THREAD_WAITING;
    next->state = THREAD_RUNNING;
    running = next;
    thread_switch(prev, next);
    UNLOCK();
}