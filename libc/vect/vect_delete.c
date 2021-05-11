#include <vect.h>
#include "heap_macros.h"


void vect_delete(vect_t* vect)
{
    FREE(vect->buf);
    FREE(vect);
}