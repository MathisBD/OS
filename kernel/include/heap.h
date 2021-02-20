#pragma once

#include <stdint.h>

void init_heap();
void* malloc(uint32_t size);
void free(void* ptr);
