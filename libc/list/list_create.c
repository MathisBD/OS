#include <list.h>
#ifdef __is_libk
#include "memory/kheap.h"
#endif

list_t* list_create()
{
    list_t* l;
    #ifdef __is_libk
    l = kmalloc(sizeof(list_t));
    #endif 
    l->first = l->last = 0;
    return l;
}