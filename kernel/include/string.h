#include <stddef.h>

extern void * memcpy(void * destination, const void * source, size_t size);   
extern void * memset(void * pointer, int value, size_t count);
extern int memcmp(const void * pointer1, const void * pointer2, size_t size);
extern void * memmove(void * destination, const void * source, size_t size);