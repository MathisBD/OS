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
#include "filesystem/ext2.h"


#define PIT_DEFAULT_FREQ 1000 // Hz


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
    init_ext2();

    // ==========
    

    /*uint32_t buf1[BLOCK_SIZE / 4];
    memset(buf1, 0, BLOCK_SIZE);

    io_request_t first_req;
    first_req.type = IO_REQ_READ;
    first_req.status = IO_REQ_WAITING;
    first_req.block_num = 0;
    first_req.data = buf1;
    first_req.data_bytes = 1024;
    first_req.next = 0;


    uint32_t buf2[BLOCK_SIZE / 4];
    memset(buf2, 0x42, BLOCK_SIZE);

    io_request_t second_req;
    second_req.type = IO_REQ_WRITE;
    second_req.status = IO_REQ_WAITING;
    second_req.block_num = 0;
    second_req.data = buf2;
    second_req.data_bytes = 1024;
    second_req.next = 0;


    uint32_t buf3[BLOCK_SIZE / 4];
    memset(buf3, 0, BLOCK_SIZE);

    io_request_t third_req;
    third_req.type = IO_REQ_READ;
    third_req.status = IO_REQ_WAITING;
    third_req.block_num = 0;
    third_req.data = buf3;
    third_req.data_bytes = 1024;
    third_req.next = 0;

    printf("first request\n");
    ata_pio_request(&first_req);

    printf("second request\n");
    ata_pio_request(&second_req);

    printf("third request\n");
    ata_pio_request(&third_req);

    for (int i = 0; i < 16; i++) {
        printf("%x ", buf1[i]);
    }
    printf("\n\n");

    for (int i = 0; i < 16; i++) {
        printf("%x ", buf2[i]);
    }
    printf("\n\n");

    for (int i = 0; i < 16; i++) {
        printf("%x ", buf3[i]);
    }
    printf("\n\n");*/


    while (1) {

    }
}