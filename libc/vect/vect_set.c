#include <vect.h>


void* vect_set(vect_t* vect, uint32_t i, void* val)
{
    if (i >= vect->size) {
        return 0;
    }
    void* prev = vect->buf[i];
    vect->buf[i] = val;
    return prev;
}
