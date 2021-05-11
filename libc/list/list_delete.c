#include <list.h>
#include "heap_macros.h"


void list_delete(list_t* l)
{
    list_node_t* node = l->first;
    while(node != 0) {
        list_node_t* next = node->next;
        FREE(node);
        node = next;
    }
    FREE(l);
}