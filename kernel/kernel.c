#include "drivers/pic_driver.h"
#include "drivers/vga_driver.h"
#include "drivers/keyboard_driver.h"
#include "drivers/pit_driver.h"
#include "tables/idt.h"
#include "tables/gdt.h"
#include "scheduler/timer.h"
#include "memory/paging.h"
#include "memory/heap.h"
#include "multiboot.h"
#include "utils/string_utils.h"
#include "memory/constants.h"
#include "loader/loader.h"
#include <stdio.h>
#include <string.h>
#include "drivers/ata_driver.h"
#include "scheduler/timer.h"
#include "filesystem/ext2/miniext.h"

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

    // ==========

    uint32_t buf1[BLOCK_SIZE / 4];
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
    printf("\n\n");
    /*uint8_t target[1024];
    memset(target, 0x00, 1024 * sizeof(uint32_t));

    read_sectors_ATA_PIO(target, 0x0, 1);
    
    printf("READ\n");

    int i;
    i = 0;
    while(i < 64)
    {
        printf("%x ", target[i] & 0xFF);
        printf("%x ", (target[i] >> 8) & 0xFF);
        i++;
    }

    printf("\n\n");
    printf("writing 0...\n\n");
    char bwrite[512];
    for(i = 0; i < 512; i++)
    {
        bwrite[i] = 0x0;
    }
    write_sectors_ATA_PIO(0x0, 2, bwrite);


    printf("reading...\n\n");
    read_sectors_ATA_PIO(target, 0x0, 1);
    
    i = 0;
    while(i < 64)
    {
        printf("%x ", target[i] & 0xFF);
        printf("%x ", (target[i] >> 8) & 0xFF);
        i++;
    }*/



    /*vga_print("mod count=");
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
        
        bool res = load_elf((char*)v_mod_start, (char*)v_mod_end);
        if (res) {
            vga_print("good\n");
        }
        else {
            vga_print("bad\n");
        }
    }*/
   
    while (1) {

    }
}