#include <stdint.h>
#include "memory.h"
#include "multiboot.h"
#include "string_utils.h"
#include "vga_driver.h"
#include "constants.h"
#include "bitset.h"
#include <stddef.h>

typedef struct {
    uint32_t address;
    uint32_t frame_count;
    // bitset : 1 == the frame is available
    uint64_t* avail_frames_bitset;
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
    uint32_t reserved_2 : 2; // cpu-reserved
    uint32_t unused     : 3; // unused, avaible for use
    uint32_t reserved_3 : 10;// cpu-reserved (because of PSE)
    uint32_t frame_addr : 10;// higher 10 bits of the frame physical address
} page_t;

typedef struct {
    page_t* pages[PD_SIZE];
} page_directory_t;

#define ALIGN(addr, align) (\
    ((addr) & ((align) - 1)) ? \
    (((addr) & ~((align) - 1)) + (align)) : \
    (addr) \
)

uint32_t placement_addr; // virtual address
void* palloc(uint32_t length)
{
    placement_addr = ALIGN(placement_addr, 16);
    
    // we can't allocate more than one page with placement alloc
    if (placement_addr + length >= V_KERNEL_START + PAGE_SIZE) {
        return 0;
    }

    uint32_t tmp = placement_addr;
    placement_addr += length;
    return (void*)tmp;
}

void print_mem_blocks(void)
{
    vga_print("Memory blocks\n");

    for (int i = 0; i < mem_blocks_count; i++) {
        char str[64];

        int_to_string(mem_blocks[i].address, str, 64);
        vga_print("address : ");
        vga_print(str);
        vga_print("   ");

        int_to_string(mem_blocks[i].frame_count, str, 64);
        vga_print("frame count : ");
        vga_print(str);
        vga_print("\n");
    }
}

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
    addr = ALIGN(addr, PAGE_SIZE);
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

    uint32_t size = 1 + (mem_blocks[mem_blocks_count].frame_count >> 3);
    mem_blocks[mem_blocks_count].avail_frames_bitset = (uint64_t*)palloc(size);

    extern void* memset(void*, int, size_t);
    memset(mem_blocks[mem_blocks_count].avail_frames_bitset, 0xFF, size);
    
    if (mem_blocks_count == 0) {
        // the first frame is already used by the kernel
        bitset_unset(mem_blocks[0].avail_frames_bitset, 0);
    }

    mem_blocks_count++;
}


// called directly from boot.S
void get_mmap(multiboot_info_t * mbd, unsigned int magic)
{
    extern int _kernel_rw_end;
    // I have to cast to uint32_t (and then implicitly promote to uint64_t), 
    // because casting to uint64_t would sign extend and I definitely don't want that
    placement_addr = (uint32_t)&_kernel_rw_end;
    
    // if the GRUB memory map is valid
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

// returns the physical address of the first free frame
uint32_t find_free_frame()
{
    for (int i = 0; i < mem_blocks_count; i++) {
        uint32_t index = bitset_find(mem_blocks[i].avail_frames_bitset, mem_blocks[i].frame_count);
        if (index < mem_blocks[i].frame_count) {
            return mem_blocks[i].address + index * PAGE_SIZE;
        }
    }
    vga_print("NO FREE FRAME\n");
    // TODO : no free frame
}

void alloc_page(uint32_t page_idx, uint32_t frame_addr)
{

}

void page_fault(page_fault_info_t info)
{
    vga_print("Page fault !\n");
    if (info.present) {
        vga_print("(present)\n"); /////////////////// TODO : not working
    }
    char str[64];
    int_to_string_base(info.address, str, 64, 16);
    vga_print(str);
    while (1) {
        
    }


    if (info.present) {
        uint32_t frame_addr = find_free_frame();
        uint32_t page_idx = info.address / PAGE_SIZE;
        alloc_page(page_idx, frame_addr);
    }
    else {
        // TODO : other cases
        vga_print("Unknown page fault type\n");
        while (1) {
        }
    }
}