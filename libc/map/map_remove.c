#include <map.h>
#include "memory/kheap.h"


void* map_remove(map_t* map, uint32_t id)
{
    for (list_node_t* lnode = map->first; lnode != 0; lnode = lnode->next) {
        map_node_t* mnode = lnode->contents;
        if (mnode->id == id) {
            uint32_t value = mnode->value;
            list_remove_node(map, lnode);
            kfree(lnode);
            kfree(mnode);
            return value;
        }
    }
    return 0;
}