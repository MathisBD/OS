#include <bitset.h>

static bool bit_one(void* addr, uint32_t a, uint32_t b)
{
    return (((uint64_t*)addr)[a] & (1ULL << b)) ? true : false;
}

uint32_t bitset_find_one(void * addr, uint32_t size)
{
    for (uint32_t a = 0; (a << 6) < size; a++) {
        if (((uint64_t*)addr)[a]) { 
            uint32_t b = 0;
            while (((a << 6) + b < size) && !bit_one(addr, a, b)) {
                b++;
            }
            // returns size if nothing was found
            return (a << 6) + b;
        }
    }
    return size;
}