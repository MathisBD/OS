#include "memory/kheap.h"
#include "memory/constants.h"
#include <list.h>
#include <panic.h>
#include <stdio.h>

// align nodes on 16 bytes
// nodes are also 16 bytes long,
// so malloced memory is 16-byte aligned
#define NODE_ALIGN 16

typedef struct __mem_node {
    // size of the following memory block that this node owns
    uint32_t size;
    struct __mem_node* next;
    struct __mem_node* prev;
} __attribute__((packed)) mem_node_t;

// free blocks 
static mem_node_t* first_hole;
// allocated blocks
static mem_node_t* first_block;


void detach_node(mem_node_t* node)
{
    if (node == first_hole) {
        first_hole = first_hole->next;
    }
    else if (node == first_block) {
        first_block = first_block->next;
    }
    
    mem_node_t* p = node->prev;
    mem_node_t* n = node->next;
    node->prev = node->next = 0;

    if (p != 0) {
        p->next = n;
    }
    if (n != 0) {
        n->prev = p;
    }
}

void add_hole(mem_node_t* node)
{
    node->prev = 0;
    node->next = first_hole;
    if (first_hole != 0) {
        first_hole->prev = node;
    }
    first_hole = node;
}

void add_block(mem_node_t* node)
{
    node->prev = 0;
    node->next = first_block;
    if (first_block != 0) {
        first_block->prev = node;
    }
    first_block = node;
}

void init_kheap()
{
    first_block = 0;

    first_hole = (mem_node_t*)HEAP_START;
    first_hole->size = HEAP_SIZE - sizeof(mem_node_t);
    first_hole->prev = first_hole->next = 0;
}

/*void compactify_heap()
{

}*/

mem_node_t* find_hole(uint32_t size)
{
    for (mem_node_t* hole = first_hole; hole != 0; hole = hole->next) {   
        if (size <= hole->size) {
            return hole;
        }
    }
    panic("find_hole : not enough space for malloc\n");
    return 0;
}

void print_node(mem_node_t* node)
{
    printf("addr=%x\tsize=%x\n", (uint32_t)node, node->size);
}

void print_lists()
{
    printf("FREE LIST\n");
    for (mem_node_t* hole = first_hole; hole != 0; hole = hole->next) {
        print_node(hole);
    }
    
    printf("BLOCK LIST\n");
    for (mem_node_t* block = first_block; block != 0; block = block->next) {  
        print_node(block);
    }
    printf("\n");
}

void* kmalloc(uint32_t size)
{
    // align size
    if (size & (NODE_ALIGN - 1)) {
        size &= ~(NODE_ALIGN - 1);
        size += NODE_ALIGN;   
    }

    mem_node_t* hole = find_hole(size);

    // switch from hole to block
    detach_node(hole);
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

void kfree(void* ptr)
{
    mem_node_t* node = (mem_node_t*)(ptr - sizeof(mem_node_t));
    
    // switch from block to hole
    // assumes the node is a block at the moment
    detach_node(node);
    add_hole(node);
}


