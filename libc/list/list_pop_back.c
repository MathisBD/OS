#include <list.h>
#include <panic.h>

void* list_pop_back(list_t* l)
{
    if (l->first == 0) {
        panic("can't pop from empty list\n");
        return 0;
    }

    if (l->first == l->last) {
        void* elem = l->first->contents;
        l->first = l->last = 0;
        return elem;
    }

    void* elem = l->last->contents;
    list_node_t* node = l->last->prev;
    node->next = 0;
    #ifdef __is_libk
    kfree(l->last);
    #endif
    l->last = node;
    return elem;
}