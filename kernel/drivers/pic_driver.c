#include <stdint.h>
#include "pic_driver.h"
#include "idt.h"

// command/status ports
#define PIC_1_COMM 0x20
#define PIC_2_COMM 0xA0
// data ports
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

#define BEGIN_INIT 0x11
#define END_OF_INTERRUPT 0x20

#define IRQ_PIT      0
#define IRQ_KEYBOARD 1

extern int write_io_port();
extern int read_io_port();

void init_pic_driver(void)
{
    // ICW1 : begin initialization
    write_io_port(PIC_1_COMM, BEGIN_INIT);
    write_io_port(PIC_2_COMM, BEGIN_INIT);

    // ICW2 : IDT offset (we need to remap the 16 interrupts
    // of PIC 1 and 2 because they overlap the general purpose interrupts)
    write_io_port(PIC_1_DATA, IDT_PIC_OFFSET);
    write_io_port(PIC_2_DATA, IDT_PIC_OFFSET + 8);

    // ICW3 : cascading (not used here)
    write_io_port(PIC_1_DATA, 0x00);
    write_io_port(PIC_2_DATA, 0x00);

    // ICW4 : enironment info
    write_io_port(PIC_1_DATA, 0x01);
    write_io_port(PIC_2_DATA, 0x01);
    // initialization finished

    // enable interrupts (we supply a bitmask :
    // 0 means enabled and 1 means disabled)
    //uint8_t enabled = ~((1 << IRQ_PIT) | (1 << IRQ_KEYBOARD));
    uint16_t enabled = ~(1 << IRQ_KEYBOARD);
    write_io_port(PIC_1_DATA, enabled & 0xFF);
    write_io_port(PIC_2_DATA, (enabled >> 8) & 0xFF);
}


// end of interrupt
void pic_eoi(int irq) 
{
    if (0 <= irq && irq < 8) {
        write_io_port(PIC_1_COMM, END_OF_INTERRUPT);
    }
    else if (8 <= irq && irq < 16) {
        write_io_port(PIC_1_COMM, END_OF_INTERRUPT);
        write_io_port(PIC_2_COMM, END_OF_INTERRUPT);
    }
}