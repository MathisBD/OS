#include "pic_driver.h"
#include "vga_driver.h"
#include "keyboard_driver.h"
#include "pit_driver.h"
#include "idt.h"
#include "gdt.h"
#include "timer.h"
#include "paging.h"
#include "heap.h"
#include "multiboot.h"
#include "string_utils.h"


#define PIT_DEFAULT_FREQ 1000 // Hz


void kernel_main(multiboot_info_t * mbd, unsigned int magic)
{
    // =========
    // LOW LEVEL
    // =========

    init_gdt();
    init_idt();

    // (we HAVE to initialize the pics 
    // before we enable interrupts, otherwise double fault)
    init_vga_driver();
    init_pic_driver();
    init_keyboard_driver();
    float pit_freq = init_pit(PIT_DEFAULT_FREQ);

    init_paging(mbd, magic);

    enable_interrupts();

    // ==========
    // HIGH LEVEL
    // ==========

    init_heap();
    init_timer(pit_freq);

    while (1) {

    }
}