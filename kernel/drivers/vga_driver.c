#include <stdint.h>
#include <stddef.h>
#include "vga_driver.h"


#define VGA_COLS 80
#define VGA_ROWS 25


// text output buffer
// entry format : 0xBFCC
// - B : background color
// - F : foreground color
// - C : ASCII character
uint16_t* vga_buffer;

int term_col;
int term_row;
uint8_t default_color;

inline uint16_t vga_entry(uint8_t color, char c)
{
    return (((uint16_t)color) << 8) | c;
}

inline void set_vga_buffer(int row, int col, uint8_t color, char c)
{
    const int index = VGA_COLS * row + col;
    vga_buffer[index] = vga_entry(color, c);
}

void clear_screen(void)
{
    for (int col = 0; col < VGA_COLS; col++) {
        for (int row = 0; row < VGA_ROWS; row++) {      
            set_vga_buffer(row, col, default_color, ' ');
        }
    }
}

void init_vga_driver(void)
{
    vga_buffer = (uint16_t*)0xB8000;
    term_col = 0;
    term_row = 0;
    // white text, black background
    default_color = 0x0F;

    clear_screen();
}

void vga_putc(char c)
{
    if (term_row >= VGA_ROWS) {
        term_col = 0;
        term_row = 0;
        clear_screen();
    }
    
    switch(c) {
    case '\n':
        {
            term_row++;
            term_col = 0;
            break;
        }
    default: 
        {
            set_vga_buffer(term_row, term_col, default_color, c);
            term_col++;
            break;
        }
    }

    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
}

void vga_print(const char* str) 
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        vga_putc(str[i]);
    }
}