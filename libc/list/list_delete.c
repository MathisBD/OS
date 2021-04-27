#include <list.h>
#ifdef __is_libk
#include "memory/kheap.h"
#endif

void list_delete(list_t* l)
{
    list_node_t* node = l->first;
    while(node != 0) {
        list_node_t* next = node->next;
        #ifdef __is_libk
        kfree(node);
        #endif
        node = next;
    }

    #ifdef __is_libk
    kfree(l);
    #endif
}