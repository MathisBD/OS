#include <stdint.h>
#include "gdt.h"

#define GDT_SIZE 5

typedef struct {
    uint16_t size_lowbits;
    uint16_t start_lowbits;
    uint8_t start_middlebits;
    uint8_t access;
    uint8_t granularity;
    uint8_t start_highbits;
} __attribute__((packed)) GDT_entry;

typedef struct {
    uint16_t limit;
    uint32_t start;
    uint16_t pack;
} __attribute__((packed)) GDTR_contents;

GDT_entry gdt[GDT_SIZE];

void set_gdt_entry(int index, uint32_t seg_start, 
    uint32_t seg_size, uint8_t access, uint8_t gran)
{
    gdt[index].start_lowbits = seg_start & 0xFFFF;
    gdt[index].start_middlebits = (seg_start >> 16) & 0xFF;
    gdt[index].start_highbits = (seg_start >> 24) & 0xFF;

    gdt[index].access = access;
    gdt[index].granularity = gran & 0xF0;

    gdt[index].size_lowbits = seg_size & 0xFFFF;
    gdt[index].granularity |= (seg_size >> 16) & 0x0F;
}

void init_gdt(void)
{
    extern void load_gdtr();

    // flat memory model/segmentation
    set_gdt_entry(0, 0, 0, 0, 0); // null segment
    set_gdt_entry(1, 0, 0xFFFFFFFF, 0x9A, 0xCF); // kernel code segment
    set_gdt_entry(2, 0, 0xFFFFFFFF, 0x92, 0xCF); // kernel data segment
    set_gdt_entry(3, 0, 0xFFFFFFFF, 0xFA, 0xCF); // user code segment
    set_gdt_entry(4, 0, 0xFFFFFFFF, 0xF2, 0xCF); // user data segment

    GDTR_contents gdtr;
    gdtr.limit = (sizeof(GDT_entry) * GDT_SIZE) - 1;
    gdtr.start = (uint32_t)&gdt;
    load_gdtr((uint32_t)&gdtr);
}
