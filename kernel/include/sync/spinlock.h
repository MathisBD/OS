#pragma once
#include <stdint.h>
#include <list.h>


typedef struct {
    // 0 : free
    // 1 : busy
    volatile int value;
} spinlock_t;

spinlock_t* spinlock_create();
void spinlock_delete(spinlock_t*);
void spinlock_acquire(spinlock_t*);
void spinlock_release(spinlock_t*);
