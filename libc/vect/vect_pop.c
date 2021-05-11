#include <vect.h>


void* vect_pop(vect_t* vect)
{
    void* val = vect->buf[vect->size-1];
    (vect->size)--;
    return val
}