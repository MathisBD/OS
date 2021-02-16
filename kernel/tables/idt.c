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

// registers pushed by the isrs in assembly
// and passed to the interrupt handler in c
typedef struct {
	// user data segment selector
	uint32_t ds;
	// user registers (pushed by pusha)
	uint32_t edi, esi, ebp, nothing, ebx, edx, ecx, eax;
	// pushed by the isrxx
	uint32_t intr_num, error_code;
} __attribute__((packed)) registers;

#define IDT_SIZE 256
IDT_entry IDT[IDT_SIZE];



void set_isr(uint32_t handler_addr, int index)
{
	IDT[index].offset_lowbits = handler_addr & 0xFFFF;
	IDT[index].selector = 0x08; // offset in the gdt of the kernel code descriptor
	IDT[index].zero = 0;
	IDT[index].type_attr = 0x8e; // interrupt gate
	IDT[index].offset_highbits = (handler_addr & 0xFFFF0000) >> 16;
}

void init_idt(void)
{
	extern void isr0();
	extern void isr1();
	extern void isr2();
	extern void isr3();
	extern void isr4();
	extern void isr5();
	extern void isr6();
	extern void isr7();

	extern void isr8();
	extern void isr9();
	extern void isr10();
	extern void isr11();
	extern void isr12();
	extern void isr13();
	extern void isr14();
	extern void isr15();

	extern void isr16();
	extern void isr17();
	extern void isr18();
	extern void isr19();
	extern void isr20();
	extern void isr21();
	extern void isr22();
	extern void isr23();

	extern void isr24();
	extern void isr25();
	extern void isr26();
	extern void isr27();
	extern void isr28();
	extern void isr29();
	extern void isr30();
	extern void isr31();

	extern void default_pic1_isr();
	extern void default_pic2_isr();

    extern void load_idtr();

	// we setup the 32 cpu-reserved interrupts
	set_isr((uint32_t)isr0, 0);
	set_isr((uint32_t)isr1, 1);
	set_isr((uint32_t)isr2, 2);
	set_isr((uint32_t)isr3, 3);
	set_isr((uint32_t)isr4, 4);
	set_isr((uint32_t)isr5, 5);
	set_isr((uint32_t)isr6, 6);
	set_isr((uint32_t)isr7, 7);

	set_isr((uint32_t)isr8, 8);
	set_isr((uint32_t)isr9, 9);
	set_isr((uint32_t)isr10, 10);
	set_isr((uint32_t)isr11, 11);
	set_isr((uint32_t)isr12, 12);
	set_isr((uint32_t)isr13, 13);
	set_isr((uint32_t)isr14, 14);
	set_isr((uint32_t)isr15, 15);

	set_isr((uint32_t)isr16, 16);
	set_isr((uint32_t)isr17, 17);
	set_isr((uint32_t)isr18, 18);
	set_isr((uint32_t)isr19, 19);
	set_isr((uint32_t)isr20, 20);
	set_isr((uint32_t)isr21, 21);
	set_isr((uint32_t)isr22, 22);
	set_isr((uint32_t)isr23, 23);

	set_isr((uint32_t)isr24, 24);
	set_isr((uint32_t)isr25, 25);
	set_isr((uint32_t)isr26, 26);
	set_isr((uint32_t)isr27, 27);
	set_isr((uint32_t)isr28, 28);
	set_isr((uint32_t)isr29, 29);
	set_isr((uint32_t)isr30, 30);
	set_isr((uint32_t)isr31, 31);

	// ignore all PIC interrupts for now
	for (int i = IDT_USER_OFFSET; i < IDT_USER_OFFSET + 16; i++) {
		uint32_t address;
		if (i < IDT_USER_OFFSET + 8) {
			address = (uint32_t)default_pic1_isr;
		}
		else {
			address = (uint32_t)default_pic2_isr;
		}
		set_isr(address, i);
	}

    IDTR_contents idtr;
	idtr.limit = sizeof(IDT_entry) * IDT_SIZE - 1;
	idtr.start = (uint32_t)&IDT;
    load_idtr((uint32_t)&idtr);
}

void interrupt_handler(registers * user_regs)
{

}