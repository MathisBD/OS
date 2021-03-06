
#include <stdint.h>
#include "vga_minidriver.h"

#define VGA_COLS 80
#define VGA_ROWS 25

uint16_t* vga_buffer = 0xB8000;
uint8_t vga_color = 0x05;
uint32_t term_col = 0;
uint32_t term_row = 0;

void set_vga_buffer(int row, int col, uint8_t color, char c)
{
    const int index = VGA_COLS * row + col;
    vga_buffer[index] = (((uint16_t)color) << 8) | c;
}

void clear_screen(void)
{
    for (int col = 0; col < VGA_COLS; col++) {
        for (int row = 0; row < VGA_ROWS; row++) {      
            set_vga_buffer(row, col, vga_color, ' ');
        }
    }
}

void print_char(char c)
{
    if (term_row >= VGA_ROWS) {
        term_col = 0;
        term_row = 0;
        clear_screen();
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

void print_string(const char* str)
{
    int i = 0;
    while (str[i]) {
        print_char(str[i]);
        i++;
    }
} 