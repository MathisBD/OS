#include <str.h>


void str_add_char(str_t* str, char c)
{
    if (str->size >= str->capacity) {
        str_grow(str, str->size + 1);
    }
    str->buf[str->size-1] = c;
    (str->size)++;
}