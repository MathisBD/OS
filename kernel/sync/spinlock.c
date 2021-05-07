#include "sync/spinlock.h"
#include <panic.h>
#include "memory/kheap.h"
#include <stdio.h>


spinlock_t* spinlock_create()
{
    spinlock_t* lock = kmalloc(sizeof(spinlock_t));
    lock->value = 0;
    return lock;
}

void spinlock_delete(spinlock_t* lock)
{
    kfree(lock);
}

void spinlock_acquire(spinlock_t* lock)
{
    while (__sync_lock_test_and_set(&(lock->value), 1)) { 
        while (lock->value);        
    }   
}

void spinlock_release(spinlock_t* lock)
{
    __sync_lock_release(&(lock->value));
}
