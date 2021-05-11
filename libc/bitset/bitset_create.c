#include <bitset.h>
#include <string.h>
#include "heap_macros.h"


void* bitset_create(uint32_t size)
{
    uint32_t bytes = size / 8;
    if (size & (8-1)) {
        bytes++; 
    }
    void* bitset = MALLOC(bytes);
    memset(bitset, 0, bytes);
    return bitset;
}