#include <list.h>
#include "heap_macros.h"


void list_add_front(list_t* l, void* elem) {
    list_node_t* node;
    node = MALLOC(sizeof(list_node_t));
    node->contents = elem;
    node->prev = 0;
    node->next = l->first;
    
    if (l->first == 0) {
        l->first = l->last = node;
    }
    else {
        l->first->prev = node;
        l->first = node;
    }
}
