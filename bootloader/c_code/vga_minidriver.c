
#include <stdint.h>
#include "vga_minidriver.h"

#define VGA_COLS 80
#define VGA_ROWS 25

uint16_t* vga_buffer = (uint16_t*)0xB8000;
uint8_t vga_color = 0x0F;
uint32_t term_col = 0;
uint32_t term_row = 0;

char digits[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F'
};

void set_vga_buffer(int row, int col, uint8_t color, char c)
{
    const int index = VGA_COLS * row + col;
    vga_buffer[index] = (((uint16_t)color) << 8) | c;
}

void vga_clear_screen(void)
{
    for (int col = 0; col < VGA_COLS; col++) {
        for (int row = 0; row < VGA_ROWS; row++) {      
            set_vga_buffer(row, col, vga_color, ' ');
        }
    }
}

void vga_print_char(char c)
{
    if (term_row >= VGA_ROWS) {
        term_col = 0;
        term_row = 0;
        vga_clear_screen();
    }
    
    switch(c) {
    case '\n':
        term_row++;
        term_col = 0;
        break;
    default:     
        set_vga_buffer(term_row, term_col, vga_color, c);
        term_col++;
        break;
    }

    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
}

void vga_print_string(const char* str)
{
    int i = 0;
    while (str[i]) {
        vga_print_char(str[i]);
        i++;
    }
} 

void vga_print_hex_byte(uint8_t byte)
{
    vga_print_char(digits[(byte >> 4) & 0x0F]);
    vga_print_char(digits[byte & 0x0F]);
}

void vga_print_mem(const void* mem, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        uint8_t byte = *((uint8_t*)(mem + i));
        vga_print_hex_byte(byte);
        
        if (i % 16 == 15) {
            vga_print_char('\n');
        }
        else if (i % 8 == 7) {
            vga_print_string("   ");
        }
        else {
            vga_print_char(' ');
        }
    }
    vga_print_char('\n');
}