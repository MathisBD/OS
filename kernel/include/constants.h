#pragma once


// kernel memory layout (in virtual address space) :
// 3GB -> 3GB+1MB : intel stuff (mapped to 0 -> 1MB)
// 3GB+1MB -> 3GB+4MB : kernel code, static data and stack,
//     followed by placement alloc (when malloc doesn't work yet)
// 3GB+4BM -> 3GB+4MB+HEAP_SIZE : kernel heap

// the kernel virtual start adress (higher half kernel)
// here is 3GB
#define V_KERNEL_START 0xC0000000 // 3GB
// beggining of the available memory (below is reserved for x86 stuff)
#define P_AVAILABLE_START 0x00100000 // 1MB
#define V_AVAILABLE_START 0xC0100000
// VGA memory buffer
#define P_VGA_ADDRESS 0x000B8000
#define V_VGA_ADDRESS 0xC00B8000

#define STACK_SIZE 0x4000 // 16KB

// the first page is reserved for static data and placement malloc
#define HEAP_START (V_KERNEL_START + PAGE_SIZE)
// we can have at most 256 - 1 = 255 heap pages, 
// we use less and will adjust it as needed
#define HEAP_SIZE (32 * PAGE_SIZE)


#define PAGE_SIZE 0x400000 // 4MB
// number of pages in a page directory
#define PD_SIZE 1024