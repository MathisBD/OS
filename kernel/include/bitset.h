#pragma once

#include <stdint.h>

void bitset_set(uint64_t* addr, uint32_t index);

void bitset_unset(uint64_t* addr, uint32_t index);

// returns the index of the first bit set (==1)
// size is the total number of bits in the bitset.
// returns size if no bit is set
uint32_t bitset_find(uint64_t* addr, uint32_t size);