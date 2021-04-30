#include "loader/elf.h"
#include <stdbool.h>
#include <string.h>
#include "tables/gdt.h"
#include "memory/constants.h"
#include <stdio.h>
#include "filesystem/fs.h"
#include "interrupts/interrupts.h"

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


void* load_program(char* prog_name)
{
    uint32_t file;
    int r = find_inode(prog_name, &file);
    if (r < 0) {
        panic("load_program : couldn't find program to load");
    }

    Elf32_Ehdr* e_hdr = kmalloc(sizeof(Elf32_Ehdr));
    int r = read_file(file, 0, sizeof(Elf32_Ehdr), e_hdr);
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
        int r = read_file(file, 
            e_hdr->e_phoff + i * e_hdr->e_phentsize, 
            sizeof(Elf32_Phdr), 
            header);
        if (r < 0) {
            panic("load_program : couldn't read segment header");
        }
        
        if (header->p_type == PT_LOAD) {
            // load the segment
            int r = read_file(file, header->p_offset, header->p_filesz, header->p_vaddr);
            if (r < 0) {
                panic("load_program : couldn't load segment");
            }
            memset(header->p_vaddr + header->p_filesz, 
                0, 
                header->p_memsz - header->p_filesz);
            vga_print("found loadable segment : ");

            vga_print("\noffset=");
            vga_print_int(header->p_offset, 16);
            vga_print("\nvaddr=");
            vga_print_int(header->p_vaddr, 16);
            vga_print("\nfilesz=");
            vga_print_int(header->p_filesz, 16);
            vga_print("\nmemsz=");
            vga_print_int(header->p_memsz, 16);
            vga_print("\n");
        }
    }

    // setup a new stack so that when we return from the interrupt
    // we start at the beggining of the program.
    void* new_stack = kmalloc(KSTACK_SIZE);
    uint32_t* new_esp = new_stack + KSTACK_SIZE;

    // interrupt frame
    new_esp = ((uint32_t)new_esp) - sizeof(intr_frame_t);
    intr_frame_t* new_frame = new_esp;
    new_frame->eax = 0;
    new_frame->ebx = 0;
    new_frame->ecx = 0;
    new_frame->edx = 0;
    new_frame->edi = 0;
    new_frame->esi = 0;

    new_frame->eip = e_hdr->e_entry;
    // handle_interrupt argument
    new_esp--;

    // jump to the entry point
    uint32_t entry_addr = e_hdr->e_entry;
    
    return true;
}