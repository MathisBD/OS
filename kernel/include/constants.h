#pragma once


// memory layout

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


// paging

#define PAGE_SIZE 0x400000 // 4MB
// number of pages in a page directory
#define PD_SIZE 1024