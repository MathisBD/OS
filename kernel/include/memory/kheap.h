#pragma once

#include <stdint.h>

void init_kheap();
void* kmalloc(uint32_t size);
// the alignement value must be a multiple of 16 bytes.
void* kmalloc_aligned(uint32_t size, uint32_t align);
void kfree(void* ptr);


//void print_lists();