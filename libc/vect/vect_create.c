#include <vect.h>
#include <string.h>
#include "heap_macros.h"


vect_t* vect_create()
{
    vect_t* vect = MALLOC(sizeof(vect_t));
    vect->size = 0;
    vect->capacity = 4;
    vect->buf = MALLOC(vect->capacity * sizeof(void*));
    memset(vect->buf, 0, vect->capacity * sizeof(void*));    
    return vect;
}