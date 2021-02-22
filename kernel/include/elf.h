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
#define EI_CLASS    4
#define EI_DATA     5
#define EI_VERSION  6
#define EI_PAD      7
#define EI_NIDENT   16

// elf versions
#define EV_NONE     0
#define EV_CURRENT  1

// elf types
#define ET_NONE     0       
#define ET_REL      1       // relocatable file
#define ET_EXEC     2       // executable file
#define ET_DYN      3       // shared object file
#define ET_CORE     4       // core file
#define ET_LOPROC   0xff00  // processor specific
#define ET_HIPROC   0xffff  // processor specific

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


// special section indexes
#define SHN_UNDEF       0
#define SHN_LORESERVE   0xff00
#define SHN_LOPROC      0xff00
#define SHN_HIPROC      0xff1f
#define SHN_ABS         0xfff1
#define SHN_COMMON      0xfff2
#define SHN_HIRESERVE   0xffff

// section type
#define SHT_NULL        0 // no associated section
#define SHT_PROGBITS    1
#define SHT_SYMTAB      2
#define SHT_STRTAB      3
#define SHT_RELA        4 // relocation entries (explicit addends)
#define SHT_HASH        5 // symbol hash table
#define SHT_DYNAMIC     6 // dynamic linking info
#define SHT_NOTE        7
#define SHT_NOBITS      8 // similar to PROBITS, uses no file space
#define SHT_REL         9 // relocation entries (no explicit addends)
#define SHT_SHLIB       10// reserved, unspecified
#define SHT_DYNSYM      11
#define SHT_LOPROC      0x70000000
#define SHT_HIPROC      0x7fffffff
#define SHT_LOUSER      0x80000000
#define SHT_HIUSER      0xffffffff

// section flags
#define SHF_WRITE       0x1 // section data is writeable at runtime
#define SHF_ALLOC       0x2 // section occupies memory at runtime
#define SHF_EXECINSTR   0x4 // section contains executable instructions
#define SHF_MASKPROC    0xf0000000

// ELF section header
typedef struct {
    Elf32_Word sh_name;     // index in the sh string table
    Elf32_Word sh_type;     
    Elf32_Word sh_flags;
    Elf32_Addr sh_addr;     // runtime memory address
    Elf32_Off  sh_offset;   // offset of the section in file
    // size of the section in file 
    // (except if type==SHT_NOBITS)
    Elf32_Word sh_size;
    Elf32_Word sh_link;     // sh table index
    Elf32_Word sh_info;     
    Elf32_Word sh_addralign;
    Elf32_Word sh_entsize;
} __attribute__((packed)) Elf32_Shdr;

// segment types
#define PT_NULL     0 
#define PT_LOAD     1 // loadable segment
#define PT_DYNAMIC  2
#define PT_INTERP   3
#define PT_NOTE     4
#define PT_SHLIB    5
#define PT_PHDR     6
#define PT_LOPROC   0x70000000
#define PT_HIPROC   0x7fffffff



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

