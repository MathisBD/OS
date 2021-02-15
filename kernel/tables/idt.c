#include <stdint.h>
#include "idt.h"
#include "vga_driver.h"

typedef struct {
    uint16_t offset_lowbits;
    uint16_t selector;
    uint8_t zero;
    uint8_t type_attr;
    uint16_t offset_highbits;
} __attribute__((packed)) IDT_entry;

typedef struct {
	uint16_t limit;
	uint32_t start;
	uint16_t pack;
} __attribute__((packed)) IDTR_contents;

#define IDT_SIZE 256
IDT_entry IDT[IDT_SIZE];



void set_idt_handler(uint32_t handler_addr, int position)
{
	// only handle the case of pic interrupts for now
	if (IDT_USER_OFFSET <= position && position < IDT_USER_OFFSET + 16) {
    	IDT[position].offset_lowbits = handler_addr & 0xFFFF;
		IDT[position].selector = 0x08; // offset in the gdt of the kernel code descriptor
		IDT[position].zero = 0;
		IDT[position].type_attr = 0x8e; // interrupt gate
		IDT[position].offset_highbits = (handler_addr & 0xFFFF0000) >> 16;
	}
}

void init_idt(void)
{
    extern void load_idtr();
	extern void default_pic1_intr();
	extern void default_pic2_intr();

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

    IDTR_contents idtr;
	idtr.limit = sizeof(IDT_entry) * IDT_SIZE - 1;
	idtr.start = (uint32_t)&IDT;
    load_idtr((uint32_t)&idtr);
}
