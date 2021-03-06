#include <stdint.h>
#include "drivers/pic_driver.h"
#include "tables/idt.h"
#include "drivers/port_io.h"
#include "sync/spinlock.h"
#include "sync/queuelock.h"
#include <stdbool.h>
#include "interrupts/interrupts.h"


// command/status ports
#define PIC_1_COMM 0x20
#define PIC_2_COMM 0xA0
// data ports
#define PIC_1_DATA 0x21
#define PIC_2_DATA 0xA1

#define BEGIN_INIT 0x11
#define END_OF_INTERRUPT 0x20


#define LOCK() \
bool _old_if = set_interrupt_flag(false);

#define UNLOCK() \
set_interrupt_flag(_old_if);


// this has to be a spinlock,
// and we have to disable interrupts when we hold it,
// because interrupt handlers will want to acquire it.
// (potential deadlock if some thread holds it, and then an
// interrupt fires and the handler tries to acquire it)
//static spinlock_t* pic_lock;

// enabled interrupts bitmask :
// 0 means enabled and 1 means disabled
static uint16_t enabled_mask;

void init_pic_driver(void)
{
    //pic_lock = ksl_create();
    //pic_lock = kql_create();

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

    // ICW4 : environment info
    port_int8_out(PIC_1_DATA, 0x01);
    port_int8_out(PIC_2_DATA, 0x01);
    // initialization finished

    enabled_mask = 0x0000;
    port_int8_out(PIC_1_DATA, enabled_mask & 0xFF);
    port_int8_out(PIC_2_DATA, (enabled_mask >> 8) & 0xFF);

}

void enable_irq(uint32_t irq)
{
    LOCK();
    if (irq < 8) {
        enabled_mask &= ~(1 << irq);
        port_int8_out(PIC_1_DATA, enabled_mask & 0xFF);
    }
    else if (irq < 16) {
        enabled_mask &= ~(1 << irq);
        port_int8_out(PIC_2_DATA, (enabled_mask >> 8) & 0xFF);
    }
    UNLOCK();
}

void disable_irq(uint32_t irq)
{
    LOCK();
    if (irq < 8) {
        enabled_mask |= 1 << irq;
        port_int8_out(PIC_1_DATA, enabled_mask & 0xFF);
    }
    else if (irq < 16) {
        enabled_mask |= 1 << irq;
        port_int8_out(PIC_2_DATA, (enabled_mask >> 8) & 0xFF);
    }
    UNLOCK();
}


// end of interrupt
void pic_eoi(uint32_t irq) 
{
    LOCK();
    if (irq < 8) {
        port_int8_out(PIC_1_COMM, END_OF_INTERRUPT);
    }
    else if (irq < 16) {
        port_int8_out(PIC_1_COMM, END_OF_INTERRUPT);
        port_int8_out(PIC_2_COMM, END_OF_INTERRUPT);
    }
    UNLOCK();
}