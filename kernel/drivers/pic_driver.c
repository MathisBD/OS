#include <stdint.h>
#include "drivers/pic_driver.h"
#include "tables/idt.h"
#include "drivers/port_io.h"

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
#define IRQ_PRIMARY_ATA 14 


void init_pic_driver(void)
{
    // ICW1 : begin initialization
    port_int8_out(PIC_1_COMM, BEGIN_INIT);
    port_int8_out(PIC_2_COMM, BEGIN_INIT);

    // ICW2 : IDT offset (we need to remap the 16 interrupts
    // of PIC 1 and 2 because they overlap the general purpose interrupts)
    port_int8_out(PIC_1_DATA, IDT_PIC_OFFSET);
    port_int8_out(PIC_2_DATA, IDT_PIC_OFFSET + 8);

    // ICW3 : cascading (not used here)
    port_int8_out(PIC_1_DATA, 0x00);
    port_int8_out(PIC_2_DATA, 0x00);

    // ICW4 : enironment info
    port_int8_out(PIC_1_DATA, 0x01);
    port_int8_out(PIC_2_DATA, 0x01);
    // initialization finished

    // enable interrupts (we supply a bitmask :
    // 0 means enabled and 1 means disabled)
    /*uint16_t enabled = ~(
        //(1 << IRQ_PIT) | 
        (1 << IRQ_KEYBOARD) | 
        (1 << IRQ_PRIMARY_ATA)
    );*/
    uint16_t enabled = 0x0000;
    port_int8_out(PIC_1_DATA, enabled & 0xFF);
    port_int8_out(PIC_2_DATA, (enabled >> 8) & 0xFF);
}


// end of interrupt
void pic_eoi(int irq) 
{
    if (0 <= irq && irq < 8) {
        port_int8_out(PIC_1_COMM, END_OF_INTERRUPT);
    }
    else if (8 <= irq && irq < 16) {
        port_int8_out(PIC_1_COMM, END_OF_INTERRUPT);
        port_int8_out(PIC_2_COMM, END_OF_INTERRUPT);
    }
}