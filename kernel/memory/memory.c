#include <stdint.h>
#include "memory.h"
#include "multiboot.h"
#include "string_utils.h"
#include "vga_driver.h"


typedef struct {
    uint64_t address;
    uint64_t length;
} mem_block_t;

#define MAX_MEM_BLOCKS 128
// available memory blocks (in RAM)
mem_block_t mem_blocks[MAX_MEM_BLOCKS];
int mem_blocks_count;


typedef struct {
    uint32_t present    : 1; // if set the page is present in memory
    uint32_t rw         : 1; // if set then readwrite else readonly
    uint32_t user       : 1; // if set user otherwise supervisor
    uint32_t reserved_1 : 2; // cpu-reserved
    uint32_t accessed   : 1; // if set the page has been accessed since last refresh
    uint32_t dirty      : 1; // if set the page has been written to since last refresh
    uint32_t reserved_2 : 2; // cpu-reserved
    uint32_t unused     : 3; // unused, avaible for use 
    uint32_t frame_addr : 20;// higher 20 bits of the frame physical address
} page_t;


// the size of a physical frame
#define FRAME_SIZE 4096

// number of pages in a page table
#define PT_SIZE 1024 
typedef struct {
    page_t pages[PT_SIZE];
} page_table_t;

// number of page tables in a page directory
#define PD_SIZE 1024 
typedef struct {
    page_table_t* page_tables[PD_SIZE];
} page_directory_t;


void print_mem_blocks(void)
{
    vga_print("Memory blocks\n");

    for (int i = 0; i < mem_blocks_count; i++) {
        char str[64];

        int_to_string(mem_blocks[i].address, str, 64);
        vga_print("address : ");
        vga_print(str);
        vga_print("   ");

        int_to_string(mem_blocks[i].length, str, 64);
        vga_print("length : ");
        vga_print(str);
        vga_print("\n");
    }
}

// called directly from boot.s
void get_mmap(multiboot_info_t * mbd, unsigned int magic)
{
    // if the GRUB memory map is valid
    if (mbd->flags & 0x20) {
        uint32_t buf_addr = (uint32_t)mbd->mmap_addr;
        uint32_t buf_length = (uint32_t)mbd->mmap_length;

        multiboot_memory_map_t * entry = (multiboot_memory_map_t*)buf_addr;
        mem_blocks_count = 0;
        while (entry < buf_addr + buf_length) {
            if (mem_blocks_count >= MAX_MEM_BLOCKS) {
                return;
            }
            if (entry->type == MULTIBOOT_MEMORY_AVAILABLE) {
                uint64_t addr = (uint64_t)entry->addr_lowbits; 
                addr |= ((uint64_t)entry->addr_highbits) << 32;

                uint64_t len = (uint64_t)entry->len_lowbits;
                len |= ((uint64_t)entry->len_highbits) << 32;

                // maybe don't keep blocks that start under one MB ?
                // (we never know what happens there, just better be careful)
                //uint64_t oneMB = 0x100000;
                //if (addr >= oneMB) {
                mem_blocks[mem_blocks_count].address = addr;
                mem_blocks[mem_blocks_count].length = len;
                mem_blocks_count++;
                //}
            }
            entry = (multiboot_memory_map_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
        }
    }
}

void alloc_frame(page_t * page, bool user, bool writeable)
{
    // the page already has a frame
    if (page->frame_addr != 0) {
        return;
    }
    else {
        uint32_t frame_addr = get_available_frame();
        page->frame_addr = (frame_addr >> 12);
        page->present = 1;
        page->rw = writeable ? 1 : 0;
        page->user = user ? 1 : 0;

        page->accessed = 0;
        page->dirty = 0;
    }
}

void free_frame(page_t * page)
{
    if (page->frame_addr != 0) {
        page->frame_addr = 0;
        // TODO : free the actual frame
    }
}


void init_memory(void)
{
}