#include "pic_driver.h"
#include "vga_driver.h"
#include "keyboard_driver.h"
#include "idt.h"
#include "gdt.h"
#include "paging.h"
#include "heap.h"
#include "multiboot.h"
#include "string_utils.h"


#define PRINT_ADDR(ptr)\
int_to_string_base((uint32_t)(ptr), str, 64, 16);\
vga_print(str);\
vga_print("\n");


void kernel_main(multiboot_info_t * mbd, unsigned int magic)
{
    // drivers 
    // (we HAVE to initialize the pics 
    // before we enable interrupts, otherwise double fault)
    init_vga_driver();
    init_pic_driver();

    // tables
    init_gdt();
    init_idt();

    init_keyboard_driver();

    // memory
    init_paging(mbd, magic);
    init_heap();

    while (1) {

    }
}