#include "vga_minidriver.h"
#include "ata_minidriver.h"
#include "miniloader.h"
#include <stddef.h>

#define SECTOR_SIZE 512

uint32_t block_size;
uint32_t sectors_per_block;


void s2_main(void* kernel_inode, uint32_t b_size)
{
    block_size = b_size;
    sectors_per_block = block_size / SECTOR_SIZE;

    vga_clear_screen();
    //vga_print_string("Hello my friend\n");

    load_kernel(kernel_inode);

    // should not happen
    vga_print_string("ERROR - kernel returned\n");
}