#include <str.h>
#include <string.h>
#include "heap_macros.h"

str_t* str_create()
{
    str_t* str = MALLOC(sizeof(str_t));
    str->size = 1; // contains the null terminator
    str->capacity = 4;
    str->buf = MALLOC(str->capacity);
    memset(str->buf, 0, str->capacity);
    return str;
}