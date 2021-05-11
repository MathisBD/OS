#pragma once
#include <stdint.h>

#if defined(__is_libk) || defined(__is_kernel)
#include "memory/kheap.h"
#define MALLOC  kmalloc
#define FREE    kfree
#endif 

#ifdef __is_libc
#include <user_heap.h>
#define MALLOC  malloc
#define FREE    free
#endif 

// a resizable array that contains void pointers
typedef struct {
    void** buf;
    uint32_t size;   
    uint32_t capacity;
} vect_t;


vect_t* vect_create();
void vect_delete(vect_t* vect);
void vect_grow(vect_t* vect, uint32_t cap);
void* vect_get(vect_t* vect, uint32_t i);
void vect_add(vect_t* vect, void* val);
void* vect_pop(vect_t* vect);
// returns a pointer to the underlying array
void** vect_get_array(vect_t* vect);

