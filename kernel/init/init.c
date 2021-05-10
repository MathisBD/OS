#include "init/init.h"
#include "tables/idt.h"
#include "tables/gdt.h"
#include "drivers/pic_driver.h"
#include "drivers/vga_driver.h"
#include "drivers/keyboard_driver.h"
#include "drivers/pit_driver.h"
#include "drivers/timer_driver.h"
#include "memory/paging.h"
#include "memory/kheap.h"
#include "memory/constants.h"
#include "interrupts/interrupts.h"
#include "filesystem/fs.h"
#include "threads/thread.h"
#include "threads/process.h"
#include "drivers/dev.h"
#include <stdio.h>

#define PIT_DEFAULT_FREQ 100 // Hz


// initialization state of parts of the kernel
static bool idt = false;
static bool gdt = false;
static bool drivers = false;
static bool paging = false;
static bool kheap = false;
static bool fs = false;
static bool threads = false;
static bool process = false;
static bool all = false;

bool is_idt_init()
{
    return idt;
}

bool is_gdt_init()
{
    return gdt;
}

bool is_drivers_init()
{
    return drivers;
}

bool is_paging_init()
{
    return paging;
}

bool is_kheap_init()
{
    return kheap;
}

bool is_fs_init()
{
    return fs;
}

bool is_threads_init()
{
    return threads;
}

bool is_process_init()
{
    return process;
}

bool is_all_init()
{
    return all; 
}

void init_kernel(boot_info_t* boot_info)
{
    // low level stuff, must go before everything else
    init_gdt();
    gdt = true;
    init_idt();
    idt = true;

    // we need a rudimentary vga driver early on
    start_init_vga_driver();

    // paging is early too
    init_paging(
        (mmap_entry_t*)(boot_info->mmap_addr + V_KERNEL_START), 
        boot_info->mmap_ent_count
    );
    paging = true;

    // heap is needed by everything else
    init_kheap();
    kheap = true;
    // we need a thread/process context to use locks
    init_threads();
    threads = true;
    init_process();
    process = true;
    // filesystem
    init_fs();
    fs = true;

    // other drivers.
    // we have to initialize the pics 
    // before we enable interrupts, otherwise double fault
    init_dev();
    finish_init_vga_driver();
    init_pic_driver();
    init_kbd_driver();
    float pit_freq = init_pit(PIT_DEFAULT_FREQ);
    init_timer(pit_freq);
    drivers = true;
    
    /*for (int i = 0; i < boot_info->mmap_ent_count; i++) {
        mmap_entry_t* ent = boot_info->mmap_addr + V_KERNEL_START + i * MMAP_ENT_SIZE;
        printf("base=%llx\tlength=%llx\ttype=%d\n", ent->base, ent->length, ent->type);
    }*/
    all = true;

    // all pending PIC interrupts will now arrive
    set_interrupt_flag(true);


    // KERNEL MAIN
    extern void kernel_main();
    kernel_main();
}