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
#include <asm_debug.h>



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
// first thread in the ready list (circular list).
// we can't use regular lists here, because we 
// can't use the heap (i.e. malloc/free) when switching between
// threads : interrupts are disabled during the timer interrupt,
// and aren't enabled before the thread switch is complete.
// if we use the heap in this interval, we need to acquire the 
// heap spinlock, which might be busy if an interrupted process
// was using it : deadlock.
static thread_t* ready;


// called by init_threads
// thread : data of the first thread
void sinit_threads(thread_t* thread)
{
    ready = 0;

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
    if (ready == 0) {
        ready = thread;
        thread->sched_next = thread;
        thread->sched_prev = thread;
    }
    else {
        thread_t* p = ready->sched_prev;
        thread->sched_next = ready;
        thread->sched_prev = p;
        p->sched_next = thread;
        ready->sched_prev = thread;
    }
}

// assumes the thread is in the ready list
static void remove_ready(thread_t* thread)
{
    if (thread == thread->sched_next) {
        ready = 0;
    }
    else {
        thread_t* p = thread->sched_prev;
        thread_t* n = thread->sched_next;
        p->sched_next = n;
        n->sched_prev = p;
    }
}

static thread_t* pop_ready()
{
    if (ready == 0) {
        return 0;
    }
    else if (ready == ready->sched_next) {
        thread_t* r = ready;
        ready = 0;
        return r;
    }
    else {
        thread_t* p = ready->sched_prev;
        thread_t* r = ready;
        thread_t* n = ready->sched_next;
        p->sched_next = n;
        n->sched_prev = p;
        ready = n; 
        return r;
    }
}

static thread_t* next_ready()
{
    return ready;
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

void sched_switch()
{
    LOCK();

    thread_t* prev = running;
    thread_t* next = pop_ready();
    if (next == 0) {
        UNLOCK();
        return;
    }

    // prev was in RUNNING state
    prev->state = THREAD_READY;
    add_ready(prev);
     
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
    sched_switch();
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

void sched_finish_thread_and_release(queuelock_t* lock)
{
    LOCK();
    kql_release(lock);

    thread_t* prev = running;
    thread_t* next = pop_ready();
    if (next == 0) {
        vga_print("no ready thread to switch to (sched_finish_thread_and_release)");
        while(1);
    }
    prev->state = THREAD_FINISHED;

    next->state = THREAD_RUNNING;
    running = next;
    thread_switch(prev, next);
    UNLOCK();
}

void sched_finish_proc_and_release(queuelock_t* lock)
{
    LOCK();
    kql_release(lock);

    thread_t* prev = running;
    // finish all the threads in the process.
    // no need for an exit code as no one can wait on them anymore.
    process_t* proc = curr_process();
    proc->state = PROC_DEAD;
    for (list_node_t* node = proc->threads->first; node != 0; node = node->next) {
        thread_t* thread = node->contents;
        if (thread->state == THREAD_READY) {
            remove_ready(thread);
        }
        // we don't care about removing waiting threads from their waiting queues,
        // as the process is about to be dead.
        thread->state = THREAD_FINISHED;
    }

    thread_t* next = pop_ready();
    if (next == 0) {
        vga_print("no ready thread to switch to (sched_finish_thread_and_release)");
        while(1);
    }
    next->state = THREAD_RUNNING;
    running = next;
    thread_switch(prev, next);
    UNLOCK();
}

void sched_suspend_and_release_spinlock(spinlock_t* lock)
{
    LOCK();
    ksl_release(lock);

    thread_t* prev = running;
    thread_t* next = pop_ready();
    if (next == 0) {
        vga_print("no ready thread to switch to (sched_suspend_and_release_spinlock) !");
        while(1);
    }
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
    thread_t* next = pop_ready();
    if (next == 0) {
        vga_print("no ready thread to switch to (sched_suspend_and_release_queuelock) !");
        while(1);
    }
    prev->state = THREAD_WAITING;
    next->state = THREAD_RUNNING;
    running = next;
    thread_switch(prev, next);
    UNLOCK();
}