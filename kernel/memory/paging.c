#include <stdint.h>
#include "memory/paging.h"
#include "memory/constants.h"
#include <bitset.h>
#include <stddef.h>
#include <string.h>
#include <stdio.h>
#include <panic.h>
#include "threads/scheduler.h"
#include "init/init.h"
#include "drivers/vga_driver.h"


// page used to copy physical frames
// without having to disable paging 
// (see copy_address_space).
#define COPY_PAGE (PT_SIZE-1)

typedef struct {
    uint32_t address;
    uint32_t frame_count;
    // bitset : 1 == the frame is available
    uint64_t avail_frames_bitset[MAX_FRAMES / 64]; 
} mem_block_t;

#define MAX_MEM_BLOCKS 32
// available memory blocks (in RAM)
static mem_block_t mem_blocks[MAX_MEM_BLOCKS];
static int mem_blocks_count;

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
} pt_entry_t;

// kernel page table
// has to be aligned on 4K
static pt_entry_t kernel_pt[PT_SIZE] __attribute__((aligned(4096)));


void print_mem_blocks()
{
    printf("mem blocks count = %d\n", mem_blocks_count);
    for (int i = 0; i < mem_blocks_count; i++) {
        printf("address = %x\tframe_count = %x\n", mem_blocks[i].address, mem_blocks[i].frame_count);
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

    // the first block (not counting the small block before 1MB)
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
        bitset_clear(mem_blocks[0].avail_frames_bitset, 0);
    }
    mem_blocks_count++;
}


void parse_mmap(mmap_entry_t* mmap, uint32_t mmap_ent_count)
{
    for (uint32_t i = 0; i < mmap_ent_count; i++) {
        if (mem_blocks_count >= MAX_MEM_BLOCKS) {
            return;
        }
        if (mmap[i].type == MMAP_ENT_TYPE_AVAILABLE) {
            add_mem_block(mmap[i].base, mmap[i].length);
        }
    }
}    

// transfer ownership of paging structures
// to the kernel
void setup_page_dir()
{
    // initialise page directory
    memset(kernel_pt, 0, PT_SIZE * sizeof(pt_entry_t));
    // kernel page mapped to 0x00
    uint32_t idx = V_KERNEL_START / PAGE_SIZE;
    kernel_pt[idx].present = 1;
    kernel_pt[idx].rw = 1;
    kernel_pt[idx].size = 1;
    kernel_pt[idx].frame_addr = 0;

    uint32_t pd_phys_addr = (uint32_t)(&kernel_pt) - V_KERNEL_START;
    extern void load_cr3(uint32_t);
    load_cr3(pd_phys_addr);
}

// returns the physical address of the first free frame
uint32_t find_free_frame()
{
    for (int i = 0; i < mem_blocks_count; i++) {
        uint32_t index = bitset_find_one(mem_blocks[i].avail_frames_bitset, mem_blocks[i].frame_count);
        if (index < mem_blocks[i].frame_count) {
            return mem_blocks[i].address + index * PAGE_SIZE;
        }
    }
    panic("no free frame\n");
    // TODO : no free frame
    return 0;
}

void claim_frame(uint32_t frame_addr)
{
    for (int i = 0; i < mem_blocks_count; i++) {
        if (mem_blocks[i].address <= frame_addr && 
            frame_addr <= mem_blocks[i].address + mem_blocks[i].frame_count * PAGE_SIZE) 
        {
            uint32_t index = (frame_addr - mem_blocks[i].address) / PAGE_SIZE;
            bitset_clear(mem_blocks[i].avail_frames_bitset, index);
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

void set_page_frame(pt_entry_t* pt, uint32_t page_idx, uint32_t frame_addr)
{
    memset(pt + page_idx, 0, sizeof(pt_entry_t));
    pt[page_idx].present = 1;
    pt[page_idx].rw = 1;
    pt[page_idx].size = 1;
    pt[page_idx].frame_addr = frame_addr >> 22;

    if (page_idx < (V_KERNEL_START / PAGE_SIZE)) {
        pt[page_idx].user = 1;
    }
}


void page_fault(intr_frame_t* frame, uint32_t mem_address)
{
    // 0 : non-present page
    // 1 : privilege violation
    bool present = frame->error_code & 0x01;
    // 0 : read only
    // 1 : read & write
    bool write = frame->error_code & 0x02;
    // 0 : supervisor only
    // 1 : user
    bool user = frame->error_code & 0x04;
    // 1 : the page reserved bit was set
    bool reserved = frame->error_code & 0x08;
    // 1 : caused by an instruction fetch
    bool instr_fetch = frame->error_code & 0x10;

    // page was absent
    if (!present) {
        if (!is_all_init()) {
            vga_print("[page miss]\n");
        }
        else {
            printf("[page miss] address=%x pid=%u\n", 
                mem_address, curr_process()->pid);
        }
        uint32_t page = mem_address / PAGE_SIZE;
        pt_entry_t* page_table;
        if (is_process_init()) {
            thread_t* thread = curr_thread();
            page_table = thread->process->page_table;
        }
        else {
            page_table = kernel_pt;
        }
        uint32_t frame_addr = find_free_frame();
        claim_frame(frame_addr);
        set_page_frame(page_table, page, frame_addr);
    }
    else {
        printf("[page_fault] address=%x, present=%x, write=%x, user=%x, reserved=%x, instr_fetch=%x\n",
            mem_address, present, write, user, reserved, instr_fetch);
        while(1);
    }
}

void init_paging(mmap_entry_t* mmap, uint32_t mmap_ent_count)
{   
    parse_mmap(mmap, mmap_ent_count);
    setup_page_dir();
}

void* kernel_page_table()
{
    return kernel_pt;
}

uint32_t physical_address(uint32_t vaddr)
{
    uint32_t page = vaddr / PAGE_SIZE;
    uint32_t ofs = vaddr % PAGE_SIZE;
    
    thread_t* thread = curr_thread();
    pt_entry_t* pt = thread->process->page_table;

    if (pt[page].present == 0) {
        panic("can't get physical address of unmapped virtual address");
    }
    return (pt[page].frame_addr << 22) | ofs;
}

void free_user_pages()
{
    panic("free user pages");
}

// assumes src_pt is the page table of the current process
void copy_address_space(void* _dest_pt, void* _src_pt)
{
    pt_entry_t* dest_pt = _dest_pt;
    pt_entry_t* src_pt = _src_pt;

    uint32_t first_kernel_page = V_KERNEL_START / PAGE_SIZE;
    // don't duplicate the kernel pages
    memcpy(dest_pt + first_kernel_page, 
        src_pt + first_kernel_page,
        (PT_SIZE - first_kernel_page) * sizeof(uint32_t));

    // duplicate every USED user page
    memset(dest_pt, 0, first_kernel_page * sizeof(uint32_t));
    for (uint32_t i = 0; i < first_kernel_page; i++) {
        if (src_pt[i].present) {
            uint32_t frame_addr = find_free_frame();
            claim_frame(frame_addr);
            set_page_frame(dest_pt, i, frame_addr);
            // we map the COPY page in the current page table to the new frame,
            // so we can memcpy() without carring about physical addresses.
            set_page_frame(src_pt, COPY_PAGE, frame_addr);
            memcpy(COPY_PAGE * PAGE_SIZE, i * PAGE_SIZE, PAGE_SIZE);
        }
    }
    // make sure we don't access the copy page later
    src_pt[COPY_PAGE].present = 0;
}
