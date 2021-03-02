#include "memory/bitset.h"


void bitset_set(uint64_t * addr, uint32_t index)
{
    uint32_t a = index >> 6;
    uint32_t b = index & 0x3F;
    addr[a] |= 1 << b;
}

void bitset_unset(uint64_t * addr, uint32_t index)
{
    uint32_t a = index >> 6;
    uint32_t b = index & 0x3F;
    addr[a] &= ~(1 << b);
}

uint32_t bitset_find(uint64_t * addr, uint32_t size)
{
    for (uint32_t a = 0; (a << 6) < size; a++) {
        if (addr[a]) { 
            uint32_t b = 0;
            while (((a << 6) + b < size) && !(addr[a] & (1 << b))) {
                b++;
            }
            // returns size if nothing was found
            return (a << 6) + b;
        }
    }
    return size;
}