#include "vga_minidriver.h"
#include "ata_minidriver.h"
#include "miniloader.h"
#include <stddef.h>
#include "bootloader_info.h"

#define SECTOR_SIZE 512

uint32_t block_size;
uint32_t sectors_per_block;

boot_info_t boot_info; // in the data segment


void s2_main(
    uint32_t b_size, 
    void* kernel_inode, 
    void* mmap,
    uint32_t mmap_entry_count)
{
    block_size = b_size;
    sectors_per_block = block_size / SECTOR_SIZE;

    boot_info.mmap_addr = (uint32_t)mmap;
    boot_info.mmap_ent_count = mmap_entry_count;

    vga_clear_screen();
    //vga_print_string("Hello from second stage\n");

    load_kernel(kernel_inode, &boot_info);

    // should not happen
    vga_print_string("ERROR - kernel returned\n");
}