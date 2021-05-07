#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "init/bootloader_info.h"
#include "interrupts/interrupts.h"



void print_mem_blocks();

void init_paging(mmap_entry_t* mmap, uint32_t mmap_ent_count);
void page_fault(intr_frame_t* frame, uint32_t mem_addresss);

// returns the address of the (statically allocated) kernel page table
void* kernel_page_table();
// returns the physical address corresponding to a MAPPED
// virtual address.
uint32_t physical_address(uint32_t virtual_address);
// free all pages below 3GB
void free_user_pages();
void copy_address_space(void* dest_pt, void* src_pt);
