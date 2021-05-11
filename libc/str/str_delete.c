#include <str.h>
#include "heap_macros.h"

void str_delete(str_t* str)
{
    FREE(str->buf);
    FREE(str);
}