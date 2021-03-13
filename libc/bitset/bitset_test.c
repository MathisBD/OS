#include <bitset.h>


bool bitset_test(void* addr, uint32_t index)
{
    uint32_t a = index >> 6;        // index / 64
    uint32_t b = index & (64 - 1);  // index % 64
    return (((uint64_t*)addr)[a] & (((uint64_t)1) << b)) ? true : false;
}