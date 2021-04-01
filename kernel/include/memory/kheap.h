#pragma once

#include <stdint.h>

void init_kheap();
void* kmalloc(uint32_t size);
void kfree(void* ptr);
