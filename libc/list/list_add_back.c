#include <list.h>

void list_add_back(list_t* l, void* elem) {
    list_node_t* node;
    #ifdef __is_libk
    node = kmalloc(sizeof(list_node_t));
    #endif
    node->contents = elem;
    node->next = 0;
    node->prev = l->last;
    
    if (l->first == 0) {
        l->first = l->last = node;
    }
    else {
        l->last->next = node;
        l->last = node;
    }
}