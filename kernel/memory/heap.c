#include "heap.h"
#include "constants.h"
#include "string_utils.h"
#include "vga_driver.h"

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

void print_node(mem_node_t* node)
{
    if (node) {
        char str[64];

        vga_print("addr=");
        int_to_string_base((uint32_t)node, str, 64, 16);
        vga_print(str);

        vga_print("  size=");
        int_to_string_base(node->size, str, 64, 16);
        vga_print(str);
        vga_print("\n");
    }
    else {
        vga_print("(none)\n");
    }
}

void print_lists()
{
    vga_print("FREE LIST\n");
    for (mem_node_t* hole = first_hole; hole != 0; hole = hole->next) {
        print_node(hole);
    }
    
    vga_print("BLOCK LIST\n");
    for (mem_node_t* block = first_block; block != 0; block = block->next) {
        print_node(block);
    }
    vga_print("\n");
}

void* malloc(uint32_t size)
{
    // align size
    if (size & (NODE_ALIGN - 1)) {
        size &= ~(NODE_ALIGN - 1);
        size += NODE_ALIGN;   
    }

    mem_node_t* hole = find_hole(size);

    remove_hole(hole);
    add_block(hole);

    // address of the potential new node
    uint32_t node_addr = (uint32_t)hole + sizeof(mem_node_t) + size;
    // end of the memory owned by the hole
    uint32_t end_addr = (uint32_t)hole + sizeof(mem_node_t) + hole->size;
    // node_addr and end_addr are aligned, since nodes and size are aligned
    if (node_addr + sizeof(mem_node_t) < end_addr) {
        mem_node_t* node = (mem_node_t*)node_addr;
        node->size = end_addr - (node_addr + sizeof(mem_node_t));
        hole->size = size;
        add_hole(node);
    }
    // otherwise don't change the hole size

    return (void*)((uint32_t)hole + sizeof(mem_node_t));

    // count free space (except at the end)
    // if it is big (>= % of alloc space),
    // compactify
}

void free(void* ptr)
{
    mem_node_t* node = (mem_node_t*)(ptr - sizeof(mem_node_t));
    remove_block(node);
    add_hole(node);
}


