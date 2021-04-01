#pragma once
#include <stdint.h>

// segment selectors
#define GDT_KCODE_SEL   0x08
#define GDT_KDATA_SEL   0x10
#define GDT_UCODE_SEL   0x18
#define GDT_UDATA_SEL   0x20

// requested privile level
// OR this with the segment selector
#define GDT_RPL_KERNEL  0x00
#define GDT_RPL_USER    0x03

void init_gdt(void);
void set_tss_esp(uint32_t addr);