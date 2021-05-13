#pragma once
#include <stdint.h>


void init_heap();
void* malloc(uint32_t size);
void* malloc_aligned(uint32_t size, uint32_t align);
void free(void* ptr);