#pragma once
#include <stdint.h>
#include <stdbool.h>

// creates a bitset with size bits on the heap,
// all bits initialized to 0
void* bitset_create(uint32_t size);
void bitset_set(void* addr, uint32_t index);
void bitset_clear(void* addr, uint32_t index);
// returns true if the bit at index is set (equal to one)
bool bitset_test(void* addr, uint32_t index);
// returns the index of the first bit equal to one/zero
// size is the total number of BITS in the bitset.
// returns size if no requested bit is found
uint32_t bitset_find_one(void* addr, uint32_t size);
uint32_t bitset_find_zero(void* addr, uint32_t size);