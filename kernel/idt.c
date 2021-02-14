#include <stdint.h>
#include "idt.h"
#include "vga_driver.h"

#define IDT_SIZE 256
IDT_entry IDT[IDT_SIZE];



void set_idt_handler(uint32_t handler_addr, int position)
{
	// only handle the case of pic interrupts for now
	if (IDT_USER_OFFSET <= position && position < IDT_USER_OFFSET + 16) {
    	IDT[position].offset_lowbits = handler_addr & 0xFFFF;
		IDT[position].selector = 0x08; /* KERNEL_CODE_SEGMENT_OFFSET */
		IDT[position].zero = 0;
		IDT[position].type_attr = 0x8e; /* INTERRUPT_GATE */
		IDT[position].offset_highbits = (handler_addr & 0xFFFF0000) >> 16;
	}
}

void init_idt(void)
{
    extern int load_idtr();
	extern int default_pic1_intr();
	extern int default_pic2_intr();

	// ignore all interrupts for now
	for (int i = IDT_USER_OFFSET; i < IDT_USER_OFFSET + 16; i++) {
		uint32_t address;
		if (i < IDT_USER_OFFSET + 8) {
			address = (uint32_t)default_pic1_intr;
		}
		else {
			address = (uint32_t)default_pic2_intr;
		}
		set_idt_handler(address, i);
	}

    // IDTR : bytes 0 and 1 contain the IDT size,
    // bytes 2 to 5 contain the IDT address
    uint32_t address = (uint32_t)IDT;
    uint32_t idtr_contents[2];
    idtr_contents[0] = (IDT_SIZE * sizeof(IDT_entry)) + ((address & 0xFFFF) << 16);
    idtr_contents[1] = address >> 16;
    load_idtr(idtr_contents);
}
