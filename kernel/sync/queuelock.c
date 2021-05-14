#include "sync/queuelock.h"
#include "memory/kheap.h"
#include "threads/thread.h"
#include "threads/scheduler.h"
#include <panic.h>
#include <stdio.h>

queuelock_t* kql_create()
{
    queuelock_t* lock = kmalloc(sizeof(queuelock_t));
    lock->value = 0;
    lock->waiting = list_create();
    lock->spinlock = ksl_create();
    lock->owner = 0;
    return lock;
}

void kql_delete(queuelock_t* lock)
{
    ksl_acquire(lock->spinlock);

    /*if (!list_empty(lock->waiting)) {
        panic("can't delete a queuelock if threads are waiting on it");
    }*/
    list_delete(lock->waiting);
    ksl_delete(lock->spinlock);
    kfree(lock);
}

void kql_acquire(queuelock_t* lock)
{
    ksl_acquire(lock->spinlock);

    // we are the owner of the lock : increment the value
    if (lock->value > 0 && lock->owner == curr_thread()->tid) {
        lock->value++;
    }
    // we are not the owner : wait for the lock to be free
    else {
        while (lock->value == 1) {
            list_add_back(lock->waiting, curr_thread());
            sched_suspend_and_release_spinlock(lock->spinlock);
            ksl_acquire(lock->spinlock);
        }
        lock->value = 1;
        lock->owner = curr_thread()->tid;
    }
    ksl_release(lock->spinlock);
}

void kql_release(queuelock_t* lock)
{
    ksl_acquire(lock->spinlock);

    if (lock->value == 0) {
        panic("can't release a queuelock that is free");
    }
    if (lock->owner != curr_thread()->tid) {
        panic("only a queuelock's owner can release it");
    }
    lock->value--;
    
    // the lock is now free : wake up a waiting thread
    if (lock->value == 0) {
        if (!list_empty(lock->waiting)) {
            thread_t* next = list_pop_front(lock->waiting);
            sched_wake_up(next);
        }
    }
    ksl_release(lock->spinlock);
}

bool kql_is_held(queuelock_t* lock)
{
    ksl_acquire(lock->spinlock);
    bool held = (lock->value > 0 && lock->owner == curr_thread()->tid);
    ksl_release(lock->spinlock);
    return held;
}