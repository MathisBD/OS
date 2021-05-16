#include <vect.h>
#include <string.h>
#include "heap_macros.h"


void vect_grow(vect_t* vect, uint32_t cap)
{
    if (cap <= vect->capacity) {
        return;
    }

    while (vect->capacity < cap) {
        vect->capacity *= 2;
    }
    void* new_buf = MALLOC(vect->capacity * sizeof(void*));
    memset(new_buf, 0, vect->capacity * sizeof(void*));
    memcpy(new_buf, vect->buf, vect->size * sizeof(void*));
    FREE(vect->buf);
    vect->buf = new_buf;
}