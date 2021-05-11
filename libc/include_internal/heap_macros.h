#pragma once

#ifdef __is_libk
#include "memory/kheap.h"
#define MALLOC  kmalloc
#define FREE    kfree
#define MALLOC_ALIGNED kmalloc_aligned
#endif 

#ifdef __is_libc
#include <user_heap.h>
#define MALLOC  malloc
#define FREE    free
#define MALLOC_ALIGNED malloc_aligned
#endif 
