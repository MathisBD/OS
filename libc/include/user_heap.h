#pragma once
#include <stdint.h>


#if defined(__is_kernel)
void init_heap(uint32_t heap_start, uint32_t heap_size);
#endif 

void* malloc(uint32_t size);
void* malloc_aligned(uint32_t size, uint32_t align);
void free(void* ptr);