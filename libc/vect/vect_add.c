#include <vect.h>


void vect_add(vect_t* vect, void* val)
{
    vect_grow(vect, vect->size + 1);
    vect->buf[vect->size] = val;
    (vect->size)++;
}