#include <panic.h>
#include <stdio.h>

void panic(char* str)
{
    printf("PANIC : %s\n", str);
    while (1);
}