#include "drivers/pic_driver.h"
#include "drivers/vga_driver.h"
#include "drivers/keyboard_driver.h"
#include "drivers/pit_driver.h"
#include "tables/idt.h"
#include "tables/gdt.h"
#include "scheduler/timer.h"
#include "memory/paging.h"
#include "memory/heap.h"
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


#define PIT_DEFAULT_FREQ 1000 // Hz

/*void print_dir(dir_entry_t* entry)
{
    if (entry != 0) {
        printf("(name=%s ino=%u)", entry->name, entry->inode);
        if (entry->next != 0) {
            printf("-->");
            print_dir(entry->next);
        }
        else {
            printf("\n");
        }
    }
}*/

struct entry {
    int a;
    ll_part_t list;
    uint64_t b;
};


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

    init_heap();
    init_timer(pit_freq);
    init_fs();

    // =========
    // TEST CODE
    // =========

    extern uint32_t get_esp();

    // setup an execution context for process 0 (init)
    create_init_proc(&curr_proc);
    proc_desc_t* p0 = curr_proc;

    uint32_t esp = get_esp();
    printf("pid=%u  stack=%x  saved_esp=%x  esp=%x\n", 
        get_pid(), curr_proc->kstack, curr_proc->ctx.esp, esp);

    // switch to p1
    proc_desc_t* p1 = copy_proc(p0, 0);
    p1->pid = new_pid();
    
    switch_proc(p0, p1);

    printf("pid=%u  stack=%x  saved_esp=%x  esp=%x\n", 
        get_pid(), curr_proc->kstack, curr_proc->ctx.esp, esp);

    
    while (1);
}