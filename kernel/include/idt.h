#pragma once
#include <stdint.h>


// the start of user-defined interrupts 
// below this is reserved
#define IDT_USER_OFFSET 0x20


void init_idt(void);

void set_isr(uint32_t handler_addr, int position);