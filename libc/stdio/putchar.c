#include <stdio.h>
#include <user_file.h>

int putchar(int ic)
{
    write(FD_STDOUT, &ic, 1);
    return ic;
}