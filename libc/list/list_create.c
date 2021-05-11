#include <list.h>
#include "heap_macros.h"


list_t* list_create()
{
    list_t* l;
    l = MALLOC(sizeof(list_t));
    l->first = l->last = 0;
    return l;
}