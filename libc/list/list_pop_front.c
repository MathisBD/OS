#include <list.h>
#include <panic.h>
#include <stdio.h>
#include "heap_macros.h"


void* list_pop_front(list_t* l)
{
    if (l->first == 0) {
        panic("can't pop from empty list");
        return 0;
    }

    if (l->first == l->last) {
        void* elem = l->first->contents;
        l->first = l->last = 0;
        return elem;
    }

    void* elem = l->first->contents;
    list_node_t* node = l->first->next;
    node->prev = 0;
    FREE(l->first);
    l->first = node;
    return elem;
}