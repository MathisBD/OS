#pragma once
#include <stdint.h>
#include <list.h>


typedef struct {
    // 0 : free
    // 1 : busy
    volatile int value;
} spinlock_t;

spinlock_t* ksl_create();
void ksl_delete(spinlock_t*);
void ksl_acquire(spinlock_t*);
void ksl_release(spinlock_t*);
