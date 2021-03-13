#pragma once
#include <stdint.h>


void bitset_set(void* addr, uint32_t index);
void bitset_clear(void* addr, uint32_t index);
// returns the index of the first bit equal to one/zero
// size is the total number of BITS in the bitset.
// returns size if no requested bit is found
uint32_t bitset_find_one(void* addr, uint32_t size);
uint32_t bitset_find_zero(void* addr, uint32_t size);