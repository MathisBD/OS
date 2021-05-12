#include "loader/elf.h"
#include <stdbool.h>
#include <string.h>
#include "tables/gdt.h"
#include "memory/constants.h"
#include "memory/kheap.h"
#include <stdio.h>
#include "filesystem/fs.h"
#include "interrupts/interrupts.h"
#include <panic.h>

// memory layout :
// ============ TOP (4GB)
// KERNEL SPACE
// ============ 3GB
// STACK
// |
// v
// 
//
// ^
// |
// HEAP
// ===========
// BSS, DATA, TEXT
// =========== BOTTOM (0GB)

#define STACK_TOP  V_KERNEL_START

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


void load_program(char* prog_name, uint32_t* entry_addr, uint32_t* user_stack_top)
{
    uint32_t file;
    int r = find_inode(prog_name, &file);
    if (r < 0) {
        panic("load_program : couldn't find program to load");
    }

    Elf32_Ehdr* e_hdr = kmalloc(sizeof(Elf32_Ehdr));
    r = read_file(file, 0, sizeof(Elf32_Ehdr), e_hdr);
    if (r < 0) {
        panic("load_program : couldn't read elf header");
    }
    if (!valid_header(e_hdr)) {
        panic("load_program : not an elf file");
    }
    // we can only load executable files
    if (e_hdr->e_type != ET_EXEC) {
        panic("load_program : not an executable file");
    }

    // load segments
    for (int i = 0; i < e_hdr->e_phnum; i++) {
        Elf32_Phdr* header = kmalloc(sizeof(Elf32_Phdr));
        r = read_file(file, 
            e_hdr->e_phoff + i * e_hdr->e_phentsize, 
            sizeof(Elf32_Phdr), 
            header);
        if (r < 0) {
            panic("load_program : couldn't read segment header");
        }
        if (header->p_type == PT_LOAD) {
            // load the segment
            r = read_file(file, header->p_offset, header->p_filesz, header->p_vaddr);
            if (r < 0) {
                panic("load_program : couldn't load segment");
            }
            memset(header->p_vaddr + header->p_filesz, 
                0, 
                header->p_memsz - header->p_filesz);
                
            //printf("segment: offset=%x, vaddr=%x, filesize=%x, memsize=%x\n",
            //    header->p_offset, header->p_vaddr, header->p_filesz, header->p_memsz);
        }
    }
    *entry_addr = e_hdr->e_entry;
    *user_stack_top = STACK_TOP;
}