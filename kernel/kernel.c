#include "drivers/pic_driver.h"
#include "drivers/vga_driver.h"
#include "drivers/keyboard_driver.h"
#include "drivers/pit_driver.h"
#include "tables/idt.h"
#include "tables/gdt.h"
#include "drivers/timer_driver.h"
#include "memory/paging.h"
#include "memory/kheap.h"
#include "bootloader_info.h"
#include "utils/string_utils.h"
#include "memory/constants.h"
#include "loader/loader.h"
#include <stdio.h>
#include <string.h>
#include "drivers/ata_driver.h"
#include "filesystem/fs.h"
#include "filesystem/ext2/ext2.h"
#include <bitset.h>
#include <linkedlist.h>
#include "threads/thread.h"
#include "threads/scheduler.h"
#include "interrupts/interrupts.h"

#define PIT_DEFAULT_FREQ 1000 // Hz


#define DELAY() {for (int i = 0; i < 100000000; i++);}


void fn(int arg)
{
    printf("thread A\n");
    DELAY(); 
    printf("thread B\n");
    DELAY();
    printf("thread C\n");
}



void kernel_main(boot_info_t* boot_info)
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
    
    /*for (int i = 0; i < boot_info->mmap_ent_count; i++) {
        mmap_entry_t* ent = boot_info->mmap_addr + V_KERNEL_START + i * MMAP_ENT_SIZE;
        printf("base=%llx\tlength=%llx\ttype=%d\n", ent->base, ent->length, ent->type);
    }*/

    init_paging(
        (mmap_entry_t*)(boot_info->mmap_addr + V_KERNEL_START), 
        boot_info->mmap_ent_count
    );
    enable_interrupts();

    // ==========
    // HIGH LEVEL
    // ==========

    init_kheap();
    init_timer(pit_freq);
    init_fs();
    init_threads();
    init_scheduler();

    // =========
    // TEST CODE
    // =========

    // create a thread
    int arg = 42;
    tid_t tid = thread_create(fn, arg);

    printf("main A\n");
    DELAY(); 
    printf("main B\n");
    DELAY();
    printf("main C\n");

    while (1);
}