#pragma once
#include <stdint.h>


// the start of user-defined interrupts 
// below this is reserved
#define IDT_USER_OFFSET 0x20

typedef struct {
    uint16_t offset_lowbits;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_highbits;
} IDT_entry;


void init_idt(void);

void set_idt_handler(uint32_t handler_addr, int position);