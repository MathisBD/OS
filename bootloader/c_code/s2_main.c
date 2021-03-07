#include "vga_minidriver.h"
#include "ata_minidriver.h"




void s2_main()
{
    vga_clear_screen();
    vga_print_string("Hello my frieand\n");

    // read
    uint8_t read_buf[512];
    if (ata_read_sector(0x000, read_buf) != 0) {
        vga_print_string("FAILED READ 1\n");
    }
    for (int i = 0; i < 32; i++) {
        vga_print_hex_byte(read_buf[i]);
    }
    vga_print_char('\n');

    // overwrite 
    uint8_t write_buf[512];
    write_buf[0] = 0x42;
    if (ata_write_sector(0x000, write_buf) != 0) {
        vga_print_string("FAILED WRITE\n");
    }

    // read again
    if (ata_read_sector(0x000, read_buf) != 0) {
        vga_print_string("FAILED READ 2\n");
    }
    for (int i = 0; i < 32; i++) {
        vga_print_hex_byte(read_buf[i]);
    }
}