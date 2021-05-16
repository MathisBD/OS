#include <vect.h>


void* vect_pop(vect_t* vect)
{
    if (vect->size == 0) {
        return 0;
    }
    void* val = vect->buf[vect->size-1];
    (vect->size)--;
    return val;
}