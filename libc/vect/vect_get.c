#include <vect.h>


void* vect_get(vect_t* vect, uint32_t i)
{
    if (i >= vect->size) {
        return 0;
    }
    return vect->buf[i];
}