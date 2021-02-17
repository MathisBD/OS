#include "pic_driver.h"
#include "vga_driver.h"
#include "keyboard_driver.h"
#include "idt.h"
#include "gdt.h"
#include "memory.h"
   


void kernel_main(void)
{
    // tables
    init_gdt();
    init_idt();

    // memory (the memory map has already been loaded
    // before kernel_main even started)
    init_memory();

    // drivers
    init_vga_driver();
    init_pic_driver();
    init_keyboard_driver();

    print_mem_blocks();

    while (1) {

    }
}