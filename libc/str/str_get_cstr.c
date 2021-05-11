#include <str.h>


char* str_get_cstr(str_t* str)
{
    return str->buf;
}