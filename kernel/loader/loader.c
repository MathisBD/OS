#include "elf.h"
#include <stdbool.h>
#include <string.h>
#include "vga_driver.h"
#include "heap.h"
#include "gdt.h"
#include "constants.h"

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

#define USER_STACK_TOP (V_KERNEL_START - 4)

bool valid_header(Elf32_Ehdr* e_hdr)
{
    if (e_hdr->e_ident[EI_MAG0] == 0x7f &&
        e_hdr->e_ident[EI_MAG1] == 'E' &&
        e_hdr->e_ident[EI_MAG2] == 'L' &&
        e_hdr->e_ident[EI_MAG3] == 'F')
    {
        return true;
    }
    return false;
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
            // copy the segment 
            memcpy(header->p_vaddr, 
                elf_start + header->p_offset, 
                header->p_filesz);
            memset(header->p_vaddr + header->p_filesz, 
                0, 
                header->p_memsz - header->p_filesz);
            /*vga_print("found loadable segment : ");

            vga_print("\noffset=");
            vga_print_int(header->p_offset, 16);
            vga_print("\nvaddr=");
            vga_print_int(header->p_vaddr, 16);
            vga_print("\nfilesz=");
            vga_print_int(header->p_filesz, 16);
            vga_print("\nmemsz=");
            vga_print_int(header->p_memsz, 16);
            vga_print("\n");*/
        }
    }

    for (uint8_t* ptr = 0; ptr < 10; ptr++) {
        vga_print_int((uint8_t)*ptr, 16);
        vga_print(" ");
    }

    // create a kernel stack for handling the interrupts 
    // of this process
    uint32_t stack_size = 4096;
    char* stack_bottom = malloc(stack_size);
    char* stack_top = stack_bottom + stack_size;
    set_tss_esp((uint32_t)stack_top);

    // jump to the entry point
    uint32_t entry_addr = e_hdr->e_entry;
    /*vga_print("\nentry point=");
    vga_print_int(entry_addr, 16);
    vga_print("\n");*/
    
    vga_print("\nJUMP\n\n");

    // setup user stack
    //memset((void*)(USER_STACK_TOP - 4096), 0, 4096);

    extern void loader_jump_user(uint32_t, uint32_t);
    loader_jump_user(entry_addr, USER_STACK_TOP);

    return true;
}