#pragma once
#include <list.h>
#include "sync/spinlock.h"

typedef uint32_t tid_t;

typedef struct  {
    // 0 : free
    // 1 and greater : busy
    // queuelocks support recursive locking : 
    // if a thread owns the lock, it can acquire it again (adding one to value), 
    // and will then have to release it one more time before it is free.
    volatile int value;
    list_t* waiting;
    spinlock_t* spinlock;
    // owner only makes sense when the lock is busy
    tid_t owner; 
} queuelock_t;


queuelock_t* kql_create();
void kql_delete(queuelock_t*);
void kql_acquire(queuelock_t*);
void kql_release(queuelock_t*);
// returns true if the current thread holds the lock.
bool kql_is_held(queuelock_t*);
