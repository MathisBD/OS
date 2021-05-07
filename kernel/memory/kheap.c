#include "memory/kheap.h"
#include "memory/constants.h"
#include <list.h>
#include <panic.h>
#include <stdio.h>
#include "sync/spinlock.h"

// align nodes on 16 bytes
// nodes are also 16 bytes long,
// so malloced memory is 16-byte aligned
#define NODE_ALIGN 16

typedef struct __mem_node {
    // size of the following memory block that this node owns
    uint32_t size;
    struct __mem_node* next;
    struct __mem_node* prev;
    uint32_t padding;
} __attribute__((packed)) mem_node_t;

// free blocks 
static mem_node_t* first_hole;
// allocated blocks
static mem_node_t* first_block;

// global heap lock (statically allocated).
// this has to be a spinlock to avoid infinite recursion
// when calling lock_acquire() on a queuelock.
static spinlock_t heap_spinlock;

static void detach_node(mem_node_t* node)
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

static void add_hole(mem_node_t* node)
{
    node->prev = 0;
    node->next = first_hole;
    if (first_hole != 0) {
        first_hole->prev = node;
    }
    first_hole = node;
}

static void add_block(mem_node_t* node)
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

    heap_spinlock.value = 0;
}


// shrink a node down to a new size.
// the part of the node that was cut off will become a new 
// node (if it is big enough).
// returns the address of the new node.
static mem_node_t* shrink_node(mem_node_t* node, uint32_t size)
{
    if (size > node->size) {
        return 0;
    }
    uint32_t node_addr = (uint32_t)node;
    uint32_t end = node_addr + sizeof(mem_node_t) + node->size;
    uint32_t new_node_addr = node_addr + sizeof(mem_node_t) + size;
    if (new_node_addr % NODE_ALIGN != 0) {
        new_node_addr = NODE_ALIGN * (new_node_addr / NODE_ALIGN + 1);
    }
    node->size = size;
    // check there is enough space for a new node
    if (new_node_addr + sizeof(node) >= end) {
        return 0;
    }
    // create a new node
    mem_node_t* new_node = (mem_node_t*)new_node_addr;
    new_node->next = new_node->prev = 0;
    new_node->size = end - (new_node_addr + sizeof(mem_node_t));    
    return new_node;
}

static mem_node_t* find_hole(uint32_t size, uint32_t align)
{
    for (mem_node_t* hole = first_hole; hole != 0; hole = hole->next) {   
        uint32_t start = ((uint32_t)hole) + sizeof(mem_node_t);
        uint32_t end = start + hole->size;
        uint32_t aligned_start = start;
        if (start % align != 0) {
            aligned_start = align * (start / align + 1);  
        }  
        if (aligned_start + size <= end) {
            // we found the node !
            if (start + sizeof(mem_node_t) < aligned_start) {
                // split the node
                uint32_t new_size = aligned_start - (sizeof(mem_node_t) + start);
                mem_node_t* new_hole = shrink_node(hole, new_size);
                add_hole(new_hole);
                return new_hole;
            }
            else {
                // just move the node
                detach_node(hole);
                hole = (mem_node_t*)(aligned_start - sizeof(mem_node_t));
                hole->size = end - aligned_start;
                add_hole(hole);
                return hole;
            }
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

inline void* kmalloc(uint32_t size)
{
    return kmalloc_aligned(size, NODE_ALIGN);
} 

void* kmalloc_aligned(uint32_t size, uint32_t align)
{
    spinlock_acquire(&heap_spinlock);

    if (align & (NODE_ALIGN - 1)) {
        panic("kmalloc : invalid align value!");
    }
    // align size
    if (size & (NODE_ALIGN - 1)) {
        size &= ~(NODE_ALIGN - 1);
        size += NODE_ALIGN;  
    }

    mem_node_t* hole = find_hole(size, align);

    // switch from hole to block
    detach_node(hole);
    add_block(hole);

    uint32_t start = ((uint32_t)hole) + sizeof(mem_node_t);
    if (start + size + sizeof(mem_node_t) < start + hole->size) {
        mem_node_t* new_hole = shrink_node(hole, size);
        add_hole(new_hole);
    }
    uint32_t res = ((uint32_t)hole) + sizeof(mem_node_t);

    spinlock_release(&heap_spinlock);
    return res;
}

void kfree(void* ptr)
{
    spinlock_acquire(&heap_spinlock);

    mem_node_t* node = (mem_node_t*)(ptr - sizeof(mem_node_t));
    // switch from block to hole
    // assumes the node is a block at the moment
    detach_node(node);
    add_hole(node);

    spinlock_release(&heap_spinlock);
}


