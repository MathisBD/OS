#include "panic.h"


void panic(const char* msg)
{
    printf("PANIC\t%s\n", msg);
    while (1) {
    }
}