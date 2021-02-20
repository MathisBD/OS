#include "heap.h"
#include "constants.h"

// align nodes on 16 bytes
// nodes are also 16 bytes long,
// so malloced memory is 16-byte aligned
#define NODE_ALIGN 16

typedef struct mem_node_t {
    // size of the following memory block that this node owns
    uint32_t size; 
    struct mem_node_t* next;
    struct mem_node_t* prev;
    uint32_t padding;
} __attribute__((packed)) mem_node_t;

// free blocks 
mem_node_t* first_hole;
// allocated blocks
mem_node_t* first_block;

void remove_hole(mem_node_t* node)
{
    if (node->prev) {
        node->prev->next = node->next;
    }
    else {
        first_hole = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
}

void remove_block(mem_node_t* node)
{
    if (node->prev) {
        node->prev->next = node->next;
    }
    else {
        first_block = node->next;
    }

    if (node->next) {
        node->next->prev = node->prev;
    }
}

void add_block(mem_node_t* node)
{
    node->prev = 0;
    if (first_block) {
        first_block->prev = node;
        node->next = first_block;
    }
    else {
        node->next = 0;
    }
    first_block = node;
}

void add_hole(mem_node_t* node)
{
    node->prev = 0;
    if (first_hole) {
        first_hole->prev = node;
        node->next = first_hole;
    }
    else {
        node->next = 0;
    }
    first_hole = node;
}

void init_heap()
{
    first_hole = (mem_node_t*)HEAP_START;
    first_hole->size = HEAP_SIZE - sizeof(mem_node_t);
    first_hole->next = 0;
    first_hole->prev = 0;

    first_block = 0;
}

void compactify_heap()
{

}

mem_node_t* find_hole(uint32_t size)
{
    for (mem_node_t* hole = first_hole; hole != 0; hole = hole->next) {
        if (size <= hole->size) {
            return hole;
        }
    }
    return 0;
}

void* malloc(uint32_t size)
{
    mem_node_t* hole = find_hole(size);

    remove_hole(hole);
    add_block(hole);

    // try to use the remaining space
    uint32_t remain_addr = (uint32_t)hole + sizeof(mem_node_t) + size;
    if (remain_addr & (NODE_ALIGN - 1)) {
        remain_addr &= ~(NODE_ALIGN - 1);
        remain_addr += NODE_ALIGN;
    }

    // we can use the remaining space
    if (remain_addr + sizeof(mem_node_t) < (uint32_t)hole + sizeof(mem_node_t) + hole->size) {
        mem_node_t* remain = (mem_node_t*)remain_addr;
        remain->size = (uint32_t)hole + sizeof(mem_node_t) + hole->size - 
            (remain_addr + sizeof(mem_node_t));
        add_hole(remain);
    }

    return (void*)((uint32_t)hole + sizeof(mem_node_t));

    // count free space (except at the end)
    // if it is big (>= % of alloc space),
    // compactify
}

void free(void* ptr)
{
    mem_node_t* node = (mem_node_t*)ptr;
    remove_block(node);
    add_hole(node);
}


