#include "drivers/pic_driver.h"
#include "drivers/vga_driver.h"
#include "drivers/keyboard_driver.h"
#include "drivers/pit_driver.h"
#include "tables/idt.h"
#include "tables/gdt.h"
#include "scheduler/timer.h"
#include "memory/paging.h"
#include "memory/kheap.h"
#include "bootloader_info.h"
#include "utils/string_utils.h"
#include "memory/constants.h"
#include "loader/loader.h"
#include <stdio.h>
#include <string.h>
#include "drivers/ata_driver.h"
#include "scheduler/timer.h"
#include "filesystem/fs.h"
#include "filesystem/ext2/ext2.h"
#include <bitset.h>
#include <linkedlist.h>
#include "scheduler/process.h"
#include "scheduler/scheduler.h"
#include <syscall.h>


#define PIT_DEFAULT_FREQ 1000 // Hz




void fn(void* arg)
{
    printf("arg=%d\n", *((int*)arg));
    while (1);
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

    // =========
    // TEST CODE
    // =========

    // setup an execution context for process 0 (init)
    create_init_proc(&curr_proc);

    // create a thread
    int arg = 42;
    pid_t tid = new_thread(fn, &arg, 0, NEW_THREAD_FLAGS_KERNEL);

    while (1);
}