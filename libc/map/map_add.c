#include <map.h>
#include "heap_macros.h"


void map_add(map_t* map, uint32_t id, void* value)
{
    map_node_t* node = MALLOC(sizeof(map_node_t));
    node->id = id;
    node->value = value;
    list_add_front(map, node);
}