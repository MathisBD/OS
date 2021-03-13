#include <bitset.h>

void bitset_set(void * addr, uint32_t index)
{
    uint32_t a = index >> 6;        // index / 64
    uint32_t b = index & (64 - 1);  // index % 64
    ((uint64_t*)addr)[a] |= 1ULL << b;
}