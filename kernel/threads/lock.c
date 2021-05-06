#include "threads/lock.h"
#include "interrupts/interrupts.h"
#include "threads/thread.h"
#include "threads/scheduler.h"
#include <panic.h>
#include "threads/process.h"
#include "memory/kheap.h"


#define LOCK_FREE   0
#define LOCK_BUSY   1

lock_id_t do_lock_create()
{
    bool old_if = set_interrupt_flag(false);

    process_t* proc = curr_thread()->process;
    lock_t* lock = kmalloc(sizeof(lock_t));
    lock->waiting = list_create();
    lock->state = LOCK_FREE;
    lock->owner = 0;

    lock_id_t lock_id = proc_add_lock(proc, lock);

    set_interrupt_flag(old_if);
    return lock_id;
}

void do_lock_delete(lock_id_t lock_id)
{
    bool old_if = set_interrupt_flag(false);

    process_t* proc = curr_thread()->process;
    lock_t* lock = proc_remove_lock(proc, lock_id);
    if (lock == 0) {
        panic("didn't find lock to delete");
    }
    if (!list_empty(lock->waiting)) {
        panic("can't delete lock when threads are waiting on it");
    }
    list_delete(lock->waiting);
    kfree(lock);

    set_interrupt_flag(old_if);
}

void do_lock_acquire(lock_id_t lock_id)
{
    bool old_if = set_interrupt_flag(false);

    process_t* proc = curr_thread()->process;
    lock_t* lock = proc_get_lock(proc, lock_id);
    if (lock == 0) {
        panic("didn't find lock to acquire");
    }

    thread_t* curr = curr_thread();
    if (lock->state == LOCK_FREE) {
        lock->state = LOCK_BUSY;
        lock->owner = curr;
    }
    else {
        list_add_back(lock->waiting, curr);
        thread_switch(SWITCH_WAIT);
    }

    set_interrupt_flag(old_if);
}

void do_lock_release(lock_id_t lock_id)
{
    bool old_if = set_interrupt_flag(false);

    process_t* proc = curr_thread()->process;
    lock_t* lock = proc_get_lock(proc, lock_id);
    if (lock == 0) {
        panic("didn't find lock to release");
    }

    if (lock->state == LOCK_FREE) {
        set_interrupt_flag(old_if);
        return;
    }
    if (lock->owner != curr_thread()) {
        panic("the current thread does not own the lock it wants to release");
    }

    lock->state = LOCK_FREE;
    lock->owner = 0;
    if (!list_empty(lock->waiting)) {
        thread_t* next = list_pop_front(lock->waiting);
        swake_up(next);
    }

    set_interrupt_flag(old_if);
}
