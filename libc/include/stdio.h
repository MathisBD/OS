#pragma once
#include <stddef.h>
#include <stdint.h>

#define EOF (-1)
 

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

typedef uint32_t fid_t;

int fprintf(fid_t file, const char* __restrict format, ...);
int fputchar(fid_t file, int c);

// outputs to STDOUT
int printf(const char* __restrict format, ...);
int putchar(int c);

#if defined(__is_kernel) || defined(__is_libk)
int print_mem(const void*, size_t count);
#endif 
