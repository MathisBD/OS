#pragma once

#include <stdint.h>

typedef uint32_t Elf32_Addr;
typedef uint16_t Elf32_Half;
typedef uint32_t Elf32_Off;
typedef uint32_t Elf32_Sword;
typedef uint32_t Elf32_Word;

// elf identifier
#define EI_MAG0     0
#define EI_MAG1     1
#define EI_MAG2     2
#define EI_MAG3     3
#define EI_NIDENT   16

// elf types
#define ET_NONE     0       
#define ET_EXEC     2       // executable file

// ELF header (at offset 0 in the file)
typedef struct {
    uint8_t     e_ident[EI_NIDENT];
    Elf32_Half  e_type;
    Elf32_Half  e_machine;
    Elf32_Word  e_version;
    Elf32_Addr  e_entry; // virtual address of program entry point
    Elf32_Off   e_phoff; // program header offset
    Elf32_Off   e_shoff; // section header offset
    Elf32_Word  e_flags;
    Elf32_Half  e_ehsize; // elf header size
    Elf32_Half  e_phentsize;
    Elf32_Half  e_phnum;
    Elf32_Half  e_shentsize;
    Elf32_Half  e_shnum;
    // sh table index for the (section name) string table
    Elf32_Half  e_shstrndx;
} __attribute__((packed)) Elf32_Ehdr;

// segment types
#define PT_NULL     0 
#define PT_LOAD     1 // loadable segment

// segment flags
#define PF_X        0x1         // execute
#define PF_W        0x2         // write
#define PF_R        0x4         // read

// ELF program header
typedef struct {
    Elf32_Word p_type;
    Elf32_Off  p_offset;
    Elf32_Addr p_vaddr;
    Elf32_Addr p_paddr;
    Elf32_Word p_filesz;
    Elf32_Word p_memsz;
    Elf32_Word p_flags;
    Elf32_Word p_align;
} __attribute__((packed)) Elf32_Phdr;

