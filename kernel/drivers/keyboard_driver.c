#include <stddef.h>

#include "keyboard_driver.h"
#include "idt.h"
#include "pic_driver.h"
#include "vga_driver.h"


// interrupt number for a keyboard interrupt
#define KEYBOARD_IRQ 1

#define KEYBOARD_COMM 0x64 // command/status port
#define KEYBOARD_DATA 0x60 // data port


void init_keyboard_driver(void)
{
    extern int keyboard_intr();

    uint32_t address = (uint32_t)keyboard_intr;
    set_idt_handler(address, IDT_USER_OFFSET + KEYBOARD_IRQ);
}

void keyboard_intr_handler(void)
{
    pic_eoi(KEYBOARD_IRQ);

	int status = read_io_port(KEYBOARD_COMM);
    // bit 1 of status tells us if the output buffer is empty/full
	if (status & 0x01) {
		int keycode = read_io_port(KEYBOARD_DATA);
    	vga_print("keyboard!\n");
	}
}
