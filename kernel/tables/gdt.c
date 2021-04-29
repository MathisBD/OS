#include <stdint.h>
#include "tables/gdt.h"
#include <string.h>

#define GDT_SIZE 6

// GDT FLAGS
#define GDT_PRESENT        0x80
// descriptor type (system OR code/data)
#define GDT_DT_CODE_DATA   0x10 
// descriptor privilege level
#define GDT_DPL_0          0x00
#define GDT_DPL_3          0x60    
// segment type
#define GDT_TYPE_DATA       0x00
#define GDT_TYPE_CODE       0x08

#define GDT_DATA_EXPANDDOWN 0x04
#define GDT_DATA_WRITEABLE  0x02
#define GDT_DATA_ACCESSED   0x01

#define GDT_CODE_CONFORMING 0x04
#define GDT_CODE_READABLE   0x02
#define GDT_CODE_ACCESSED   0x01


typedef struct {
    uint16_t size_lowbits;
    uint16_t start_lowbits;
    uint8_t start_middlebits;
    uint8_t flags;
    uint8_t granularity;
    uint8_t start_highbits;
} __attribute__((packed)) gdt_entry_t;

typedef struct {
    uint16_t limit;
    uint32_t start;
    uint16_t pack;
} __attribute__((packed)) gdtr_contents_t;

typedef struct {
    uint16_t link;
    uint16_t reserved_1;
    
    // we use esp0 and ss0
    uint32_t esp0;
    uint16_t ss0;
    uint16_t reserved_2;

    uint32_t esp1;
    uint16_t ss1;
    uint16_t reserved_3;

    uint32_t esp2;
    uint16_t ss2;
    uint16_t reserved_4;

    uint32_t cr3;
    uint32_t eip;
    uint32_t eflags;
    uint32_t eax;
    uint32_t ecx;
    uint32_t edx;
    uint32_t ebx;
    uint32_t esp;
    uint32_t ebp;
    uint32_t esi;
    uint32_t edi;

    // we use the segments
    uint16_t es;
    uint16_t reserved_5;
    uint16_t cs;
    uint16_t reserved_6;
    uint16_t ss;
    uint16_t reserved_7;
    uint16_t ds;
    uint16_t reserved_8;
    uint16_t fs;
    uint16_t reserved_9;
    uint16_t gs;
    uint16_t reserved_10;

    uint16_t ldtr;
    uint16_t reserved_11;
    uint16_t reserved_12;
    uint16_t iopb_offset;
} __attribute__((packed)) tss_entry_t;


static gdt_entry_t gdt[GDT_SIZE];
static tss_entry_t tss_entry; // we use only one tss entry

void set_gdt_entry(int index, uint32_t seg_start, 
    uint32_t seg_size, uint8_t flags, uint8_t gran)
{
    gdt[index].start_lowbits = seg_start & 0xFFFF;
    gdt[index].start_middlebits = (seg_start >> 16) & 0xFF;
    gdt[index].start_highbits = (seg_start >> 24) & 0xFF;

    gdt[index].flags = flags;
    gdt[index].granularity = gran & 0xF0;

    gdt[index].size_lowbits = seg_size & 0xFFFF;
    gdt[index].granularity |= (seg_size >> 16) & 0x0F;
}

void set_tss_esp(uint32_t addr)
{
    tss_entry.esp0 = addr;
}

void init_gdt(void)
{
    extern void load_gdtr();
    extern void load_tr();

    // flat memory model/segmentation
    // order is VERY important here : we use it each time
    // we want to load a segment register
    set_gdt_entry(0, 0, 0, 0, 0); // null segment
    uint8_t kernel_flags = GDT_PRESENT | GDT_DPL_0 | GDT_DT_CODE_DATA;
    uint8_t user_flags = GDT_PRESENT | GDT_DPL_3 | GDT_DT_CODE_DATA;
    uint8_t code_flags = GDT_TYPE_CODE | GDT_CODE_READABLE;
    uint8_t data_flags = GDT_TYPE_DATA | GDT_DATA_WRITEABLE;

    set_gdt_entry(1, 0, 0xFFFFFFFF, kernel_flags | code_flags, 0xCF); 
    set_gdt_entry(2, 0, 0xFFFFFFFF, kernel_flags | data_flags, 0xCF); 
    set_gdt_entry(3, 0, 0xFFFFFFFF, user_flags | code_flags, 0xCF); 
    set_gdt_entry(4, 0, 0xFFFFFFFF, user_flags | data_flags, 0xCF);
    
    // tss entry
    uint32_t tss_start = &tss_entry;
    uint32_t tss_end = &tss_entry + sizeof(tss_entry_t);
    uint8_t tss_flags = GDT_PRESENT | GDT_TYPE_CODE | GDT_DPL_3 | GDT_CODE_ACCESSED;
    set_gdt_entry(5, tss_start, tss_end, tss_flags, 0x00);

    // initialize the tss entry
    memset(&tss_entry, 0, sizeof(tss_entry_t));
    tss_entry.ss0 = 0x10;
    tss_entry.esp0 = 0;
    // RPL is 0x03
    tss_entry.cs = 0x08 | 0x03;
    tss_entry.ss = tss_entry.ds = tss_entry.es = tss_entry.fs = tss_entry.gs = 0x10 | 0x03; 


    // load the gdt and task registers
    gdtr_contents_t gdtr;
    gdtr.limit = (sizeof(gdt_entry_t) * GDT_SIZE) - 1;
    gdtr.start = (uint32_t)&gdt;
    load_gdtr((uint32_t)&gdtr);
    load_tr(0x28 | 0x03); // 0x03 : RPL
}
