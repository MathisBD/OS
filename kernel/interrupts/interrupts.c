#include "interrupts/interrupts.h"
#include "drivers/pic_driver.h"
#include "drivers/keyboard_driver.h"
#include "drivers/ata_driver.h"
#include "scheduler/timer.h"
#include "memory/paging.h"
#include "tables/idt.h"
#include "scheduler/process.h"
#include "scheduler/scheduler.h"


void handle_interrupt(intr_stack_t* pregs)
{
	// PIC IRQs
	if (IDT_PIC_OFFSET <= pregs->intr_num && 
		pregs->intr_num < IDT_PIC_OFFSET + 16)
	{
		int irq = pregs->intr_num - IDT_PIC_OFFSET;
		pic_eoi(irq);
		
		switch(irq) {
		case 0: // clock (PIT)
			timer_interrupt(pregs->eax);
			break;
		case 1: // keyboard
			keyboard_interrupt();
			break;
		case 14: // primary ATA drive
			ata_primary_interrupt();
			break;
		}
		return;
	}

	// general interrupts
	switch(pregs->intr_num) {
	case 14: // page fault
	{
		page_fault_info_t info;
		info.present = pregs->error_code & 0x01;
		info.read_write = pregs->error_code & 0x02;
		info.user_supervisor = pregs->error_code & 0x04;
		info.reserved = pregs->error_code & 0x08;
		info.instr_fetch = pregs->error_code & 0x10;

		extern uint32_t get_cr2();
		info.address = get_cr2();

		page_fault(info);
		break;
	}
    case 128: // system call
    {
        syscall_interrupt(pregs);
        break;
    }
	default:
		printf("Uncatched interrupt !\nint num=%d\n", pregs->intr_num);
		while (1);
		break;
	}
}


void syscall_interrupt(intr_stack_t* pregs) 
{
    switch (pregs->eax) {
    case SYSCALL_FORK:
    {
		uint32_t flags = 0;
		fork(curr_proc, flags, pregs);
		break;
    }
    default:
        printf("Unknown system call !\nsyscall number=%d\n", pregs->eax);
        while(1);
        break;
    }
}