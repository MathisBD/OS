#include "memory/kheap.h"
#include "memory/constants.h"
#include <linkedlist.h>
#include <panic.h>
#include <stdio.h>

// align nodes on 16 bytes
// nodes are also 16 bytes long,
// so malloced memory is 16-byte aligned
#define NODE_ALIGN 16

typedef struct {
    // size of the following memory block that this node owns
    uint32_t size;
    // linked list this node is part of
    ll_part_t list;
} __attribute__((packed)) mem_node_t;

// free blocks 
ll_part_t holes_head;
// allocated blocks
ll_part_t blocks_head;


void init_kheap()
{
    ll_init(&holes_head);
    ll_init(&blocks_head);

    mem_node_t* first_hole = HEAP_START;
    first_hole->size = HEAP_SIZE - sizeof(mem_node_t);
    ll_add(&(first_hole->list), &holes_head);
}

/*void compactify_heap()
{

}*/

mem_node_t* find_hole(uint32_t size)
{
    ll_for_each_entry(hole, &holes_head, mem_node_t, list) {
        if (size <= hole->size) {
            return hole;
        }
    }

    panic("find_hole : not enough space for malloc\n");

    return 0;
}

void print_node(mem_node_t* node)
{
    if (node) {
        printf("addr=%x\tsize=%x\n", (uint32_t)node, node->size);
    }
    else {
        printf("(none)\n");
    }
}

void print_lists()
{
    printf("FREE LIST\n");
    ll_for_each_entry(hole, &holes_head, mem_node_t, list) {
        print_node(hole);
    }
    
    printf("BLOCK LIST\n");
    ll_for_each_entry(block, &blocks_head, mem_node_t, list) {
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
    ll_rem(&(hole->list));
    ll_add(&(hole->list), &blocks_head);

    // address of the potential new node
    uint32_t node_addr = (uint32_t)hole + sizeof(mem_node_t) + size;
    // end of the memory owned by the hole
    uint32_t end_addr = (uint32_t)hole + sizeof(mem_node_t) + hole->size;
    // node_addr and end_addr are aligned, since nodes and size are aligned
    if (node_addr + sizeof(mem_node_t) < end_addr) {
        mem_node_t* node = (mem_node_t*)node_addr;
        node->size = end_addr - (node_addr + sizeof(mem_node_t));
        hole->size = size;
        ll_add(&(node->list), &holes_head);
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
    ll_rem(&(node->list));
    ll_add(&(node->list), &holes_head);
}


