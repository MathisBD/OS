#include <str.h>


void str_delete(str_t* str)
{
    FREE(str->buf);
    FREE(str);
}