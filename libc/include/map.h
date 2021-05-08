#pragma once
#include <list.h>
#include <stdint.h>


// maps integer keys to void pointers.
typedef list_t map_t;
typedef struct {
    uint32_t id;
    void* value;
} map_node_t;


map_t* map_create();
void map_delete(map_t* map);

void map_add(map_t* map, uint32_t id, void* value);
void* map_get(map_t* map, uint32_t id);
// returns the element we removed.
void* map_remove(map_t* map, uint32_t id);
