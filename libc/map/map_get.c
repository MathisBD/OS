#include <map.h>


void* map_get(map_t* map, uint32_t id)
{
    for (list_node_t* lnode = map->first; lnode != 0; lnode = lnode->next) {
        map_node_t* mnode = lnode->contents;
        if (mnode->id == id) {
            return mnode->value;
        }
    }
    return 0;
}