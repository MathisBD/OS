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

    // drivers
    init_vga_driver();
    init_pic_driver();
    init_keyboard_driver();

    //print_mem_blocks();

    vga_print("Hello paging world\n");

    init_memory();

    while (1) {

    }
}