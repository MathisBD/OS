#pragma once
#include <stdint.h>


// the start of PIC interrupts
#define IDT_PIC_OFFSET 32

void init_idt(void);