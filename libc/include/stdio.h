#pragma once
#include <stddef.h>

#define EOF (-1)
 
#ifdef __cplusplus
extern "C" {
#endif

// format specifiers : '%[width][type]'
// width : 
// pad with leading spaces until minimum width (e.g. printf("%5d", -18) -> "  -18")
// if prefixed with a zero, pad with zeros instead (e.g. printf("%05x", 0x42) -> "00042")
// type :
// %c : character
// %s : null terminated string
// %d, %ld, %lld : int / long int / long long int -> decimal 
// %u, %lu, %llu : unsigned int / unsigned long int / unsigned long long int -> decimal
// %x, %lx, %llx : unsigned int / unsigned long int / unsigned long long int -> hexadecimal

int printf(const char* __restrict, ...);
int putchar(int);
int puts(const char*);

#ifdef __is_libk
int print_mem(const void*, size_t count);
#endif 

#ifdef __cplusplus
}
#endif
