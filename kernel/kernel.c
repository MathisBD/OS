#include "pic_driver.h"
#include "vga_driver.h"
#include "keyboard_driver.h"
#include "idt.h"


void kernel_main(void)
{
    // the vga driver is standalone
    init_vga_driver();

    // order is important
    init_pic_driver();
    init_idt();
    init_keyboard_driver();


    while (1) {

    }
}