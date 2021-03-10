#include <stdint.h>
#include "memory/paging.h"
#include "multiboot.h"
#include "memory/constants.h"
#include "memory/bitset.h"
#include <stddef.h>
#include <string.h>
#include <stdio.h>


typedef struct {
    uint32_t address;
    uint32_t frame_count;
    // bitset : 1 == the frame is available
    uint64_t avail_frames_bitset[MAX_FRAMES / 64]; 
} mem_block_t;

#define MAX_MEM_BLOCKS 128
// available memory blocks (in RAM)
mem_block_t mem_blocks[MAX_MEM_BLOCKS];
int mem_blocks_count;

// the physical address of a frame is limited to 10 + 22 = 32 bits
// we can address 4GB at most
typedef struct {
    uint32_t present    : 1; // if set the page is present in memory
    uint32_t rw         : 1; // if set then readwrite else readonly
    uint32_t user       : 1; // if set user otherwise supervisor
    uint32_t reserved_1 : 2; // cpu-reserved
    uint32_t accessed   : 1; // if set the page has been accessed since last refresh
    uint32_t dirty      : 1; // if set the page has been written to since last refresh
    uint32_t size       : 1; // must be set for 4MB pages
    uint32_t reserved_2 : 1; // cpu-reserved
    uint32_t unused     : 3; // unused, avaible for use
    uint32_t reserved_3 : 10;// cpu-reserved (because of PSE)
    uint32_t frame_addr : 10;// higher 10 bits of the frame physical address
} pde_entry_t;

// kernel page directory
// has to be aligned on 4K
pde_entry_t kernel_pd[PD_SIZE] __attribute__((aligned(4096)));


void add_mem_block(uint64_t addr, uint64_t len)
{
    uint64_t end = addr + len;

    // we only implement 32 bit addresses
    uint64_t limit = ((uint64_t)1) << 32;
    if (end >= limit) {
        end = limit;
    }
    if (addr >= end) {
        return;
    }

    // don't use small memory blocks
    // (the first one, i.e. before 1MB, will be merged
    // with the next block)
    if (addr & (PAGE_SIZE - 1)) {
        addr &= ~(PAGE_SIZE - 1);
        addr += PAGE_SIZE;
    }
    if (addr >= end) {
        return;
    } 
    if (end - addr < PAGE_SIZE) {
        return;
    }

    // the first page (not counting the small page before 1MB)
    // is extended to start at address 0
    if (mem_blocks_count == 0) {
        addr = 0;
    }

    // don't use the fractionnal page at the end of the block
    mem_blocks[mem_blocks_count].address = addr;
    mem_blocks[mem_blocks_count].frame_count = (uint32_t)((end - addr) / PAGE_SIZE);

    memset(mem_blocks[mem_blocks_count].avail_frames_bitset, 0xFF, MAX_FRAMES / 64);
    
    if (mem_blocks_count == 0) {
        // the first frame is already used by the kernel
        bitset_unset(mem_blocks[0].avail_frames_bitset, 0);
    }

    mem_blocks_count++;
}


void get_mmap(multiboot_info_t * mbd)
{
    // check the GRUB memory map is valid
    if (!(mbd->flags & 0x20)) {
        return;
    }

    uint32_t buf_addr = (uint32_t)mbd->mmap_addr + V_KERNEL_START;
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

            add_mem_block(addr, len);
        }
        entry = (multiboot_memory_map_t*)((uint32_t)entry + entry->size + sizeof(entry->size));
    }
}    

// transfer ownership of paging structures
// to the kernel
void setup_page_dir()
{
    // initialise page directory
    memset(kernel_pd, 0, PD_SIZE * sizeof(pde_entry_t));
    // kernel page mapped to 0x00
    uint32_t idx = V_KERNEL_START / PAGE_SIZE;
    kernel_pd[idx].present = 1;
    kernel_pd[idx].rw = 1;
    kernel_pd[idx].size = 1;
    kernel_pd[idx].frame_addr = 0;

    uint32_t pd_phys_addr = (uint32_t)(&kernel_pd) - V_KERNEL_START;
    extern void load_cr3(uint32_t);
    load_cr3(pd_phys_addr);
}

// returns the physical address of the first free frame
uint32_t find_free_frame()
{
    for (int i = 0; i < mem_blocks_count; i++) {
        uint32_t index = bitset_find(mem_blocks[i].avail_frames_bitset, mem_blocks[i].frame_count);
        if (index < mem_blocks[i].frame_count) {
            return mem_blocks[i].address + index * PAGE_SIZE;
        }
    }
    printf("NO FREE FRAME\n");
    // TODO : no free frame
}

void claim_frame(uint32_t frame_addr)
{
    for (int i = 0; i < mem_blocks_count; i++) {
        if (mem_blocks[i].address <= frame_addr && 
            frame_addr <= mem_blocks[i].address + mem_blocks[i].frame_count * PAGE_SIZE) 
        {
            uint32_t index = (frame_addr - mem_blocks[i].address) / PAGE_SIZE;
            bitset_unset(mem_blocks[i].avail_frames_bitset, index);
        }
    }
}

void free_frame(uint32_t frame_addr)
{
    for (int i = 0; i < mem_blocks_count; i++) {
        if (mem_blocks[i].address <= frame_addr && 
            frame_addr <= mem_blocks[i].address + mem_blocks[i].frame_count * PAGE_SIZE) 
        {
            uint32_t index = (frame_addr - mem_blocks[i].address) / PAGE_SIZE;
            bitset_set(mem_blocks[i].avail_frames_bitset, index);
        }
    }
}

void alloc_page(uint32_t idx, uint32_t frame_addr)
{
    claim_frame(frame_addr);

    kernel_pd[idx].present = 1;
    kernel_pd[idx].rw = 1;
    kernel_pd[idx].size = 1;
    kernel_pd[idx].frame_addr = frame_addr >> 22;

    //if (idx < V_KERNEL_START / PAGE_SIZE) {
        kernel_pd[idx].user = 1;
    //}
}


void page_fault(page_fault_info_t info)
{
    printf("PAGE FAULT (addr=%x)\n", info.address);

    // page was absent
    if (!info.present) {
        uint32_t frame_addr = find_free_frame();
        uint32_t page_idx = info.address / PAGE_SIZE;
        alloc_page(page_idx, frame_addr);
    }
    else {
        panic("page_fault : unknown page fault type");
    }
}

void init_paging()
{   
    setup_page_dir();
}