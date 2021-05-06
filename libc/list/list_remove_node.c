#include <list.h>


void list_remove_node(list_t* list, list_node_t* node)
{
    if (list->first == node && list->last == node) {
        list->first = list->last = 0;
    }
    else if (list->first == node) {
        list->first = node->next;
        list->first->prev = 0;
    }
    else if (list->last == node) {
        list->last = node->prev;
        list->last->next = 0;
    }
    else {
        list_node_t* n = node->next;
        list_node_t* p = node->prev;
        n->prev = p;
        p->next = n;
    }
}