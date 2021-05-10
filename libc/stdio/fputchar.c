#include <stdio.h>
#include <user_file.h>


int fputchar(fid_t file, int ic)
{
    write(file, &ic, 1);
    return ic;
}