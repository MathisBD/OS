#include "interrupts/interrupts.h"
#include "drivers/pic_driver.h"
#include "drivers/keyboard_driver.h"
#include "drivers/ata_driver.h"
#include "drivers/timer_driver.h"
#include "memory/paging.h"
#include "tables/idt.h"
#include <stdio.h>
#include <panic.h>


void handle_interrupt(intr_frame_t* frame)
{
	// PIC IRQs
	if (IDT_PIC_OFFSET <= frame->intr_num && 
		frame->intr_num < IDT_PIC_OFFSET + 16)
	{
		int irq = frame->intr_num - IDT_PIC_OFFSET;
		pic_eoi(irq);
		
		switch(irq) {
		case 0: // clock (PIT)
			timer_interrupt();
			return;
		case 1: // keyboard
			keyboard_interrupt();
			return;
		case 14: // primary ATA drive
			ata_primary_interrupt();
			return;
		default:
			printf("unknown pic irq : %u\n", irq);
			while(1);
		}
	}

	// general interrupts
	switch(frame->intr_num) {
	case 14: // page fault
	{
		extern uint32_t get_cr2();
		uint32_t faulty_address = get_cr2();

		page_fault(frame, faulty_address);
		return;
	}
	default:
		printf("Uncatched interrupt !\nint num=%d\n", frame->intr_num);
		printf("instruction : %x:%x\n", frame->cs, frame->eip);
		printf("registers : eax=%x, ebx=%x, ecx=%x, edx=%x, esi=%x, edi=%x\n",
			frame->eax, frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi);
		while(1);
	}
}

