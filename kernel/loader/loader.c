#include "elf.h"
#include <stdbool.h>

// memory layout :
// ============ TOP (4GB)
// KERNEL SPACE
// ============
// STACK
// |
// v
// 
//
// ^
// |
// HEAP
// BSS
// DATA
// TEXT
// =========== BOTTOM (0GB)

void load_segment(Elf32_Ehdr* e_hdr, Elf32_Phdr* header)
{
    
}


// returns false if the file could not be loaded
bool load_elf(char* elf_start, char* elf_end)
{
    // 1) copy the segments in the process space (0 to 3GB)
    // 2) jump to the beggining of .text

    Elf32_Ehdr* e_hdr = elf_start;

    if (!valid_header(e_hdr)) {
        return false;
    }
    // we can only load executable files
    if (e_hdr->e_type != ET_EXEC) {
        return false;
    }

    // load segments
    for (int i = 0; i < e_hdr->e_phnum; i++) {
        Elf32_Phdr* header = elf_start + e_hdr->e_phoff + i * e_hdr->e_phentsize;
        if (header->p_type == PT_LOAD) {
            load_segment(e_hdr, header);
        }
    }

    // jump to the entry point
    uint32_t entry_addr = e_hdr->e_entry;
    extern void loader_jump(uint32_t);
    loader_jump(entry_addr);
}