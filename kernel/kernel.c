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
#include "constants.h"


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

    vga_print("mod count=");
    vga_print_int(mbd->mods_count, 10);
    vga_print("\n");

    uint32_t v_mods_addr = mbd->mods_addr + V_KERNEL_START;
    for (uint32_t* ptr = v_mods_addr; ptr < v_mods_addr + 8 * mbd->mods_count; ptr += 2) {
        uint32_t v_mod_start = *ptr + V_KERNEL_START;
        uint32_t v_mod_end = *(ptr + 1) + V_KERNEL_START;

        vga_print("mod start=");
        vga_print_int(v_mod_start, 16);
        vga_print("  mod end=");
        vga_print_int(v_mod_end, 16);
        vga_print("\n");
        
        for (char* ptr = v_mod_start; ptr < v_mod_end; ptr++) {
            vga_print_int(*ptr, 16);
            vga_print(" ");
        }
    }
   
    while (1) {

    }
}