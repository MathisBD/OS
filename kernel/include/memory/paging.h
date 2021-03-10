#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "bootloader_info.h"

// info passed to the page fault handler
typedef struct {
    uint32_t address;
    // 0 : non-present page
    // 1 : privilege violation
    bool present;
    // 0 : read
    // 1 : write
    bool read_write;
    // 0 : supervisor
    // 1 : user
    bool user_supervisor;
    // 1 : the page reserved bit was set
    bool reserved;
    // 1 : caused by an instruction fetch
    bool instr_fetch;
} page_fault_info_t;


void print_mem_blocks();

void init_paging(mmap_entry_t* mmap, uint32_t mmap_ent_count);
void page_fault(page_fault_info_t info);