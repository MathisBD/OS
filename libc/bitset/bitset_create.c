#include <bitset.h>
#include <string.h>
#ifdef __is_libk
#include "memory/kheap.h"
#endif

void* bitset_create(uint32_t size)
{
    uint32_t bytes = size / 8;
    if (size & (8-1)) {
        bytes++; 
    }
#ifdef __is_libk
    void* bitset = kmalloc(bytes);
#endif 
#ifdef __is_libc
    panic("bitset_create: no libc malloc\n");
#endif
    memset(bitset, 0, bytes);
    return bitset;
}