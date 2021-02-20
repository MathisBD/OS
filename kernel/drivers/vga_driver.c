#include <stdint.h>
#include <stddef.h>
#include "vga_driver.h"
#include "constants.h"

// io ports to communicate with the vga cursor
#define CURSOR_PORT_1 0x3D4
#define CURSOR_PORT_2 0x3D5

#define VGA_COLS 80
#define VGA_ROWS 25

// sets the height of the cursor
// range : 0 (very top) to 15 (very bottom)
#define CURSOR_TOP 13
#define CURSOR_BOTTOM 15

extern int write_io_port();
extern int read_io_port();

// text output buffer
// entry format : 0xBFCC
// - B : background color
// - F : foreground color
// - C : ASCII character
uint16_t* vga_buffer;

int term_col;
int term_row;
uint8_t default_color;

uint16_t vga_entry(uint8_t color, char c)
{
    return (((uint16_t)color) << 8) | c;
}

void set_vga_buffer(int row, int col, uint8_t color, char c)
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

void enable_cursor(void)
{
    uint8_t b;

    write_io_port(CURSOR_PORT_1, 0x0A);
    b = read_io_port(CURSOR_PORT_2);
    write_io_port(CURSOR_PORT_2, (b & 0xC0) | CURSOR_TOP);

    write_io_port(CURSOR_PORT_1, 0x0B);
    b = read_io_port(CURSOR_PORT_2);
    write_io_port(CURSOR_PORT_2, (b & 0xE0) | CURSOR_BOTTOM);
}

void disable_cursor(void)
{
    write_io_port(CURSOR_PORT_1, 0x0A);
    write_io_port(CURSOR_PORT_2, 0x20);
}

void move_cursor(int row, int col)
{
    uint16_t pos = row * VGA_COLS + col;

    write_io_port(CURSOR_PORT_1, 0x0F);
    write_io_port(CURSOR_PORT_2, (uint8_t)(pos & 0xFF));

    write_io_port(CURSOR_PORT_1, 0x0E);
    write_io_port(CURSOR_PORT_2, (uint8_t)((pos & 0xFF00) >> 8));
}

void init_vga_driver(void)
{
    vga_buffer = (uint16_t*)V_VGA_ADDRESS;
    term_col = 0;
    term_row = 0;
    // white text, black background
    default_color = 0x0F;

    clear_screen();

    // setup cursor
    enable_cursor();
    move_cursor(0, 0);
}

void private_putc(char c) 
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

void vga_putc(char c)
{
    private_putc(c);
    move_cursor(term_row, term_col);
}

void vga_print(const char* str) 
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        private_putc(str[i]);
    }
    move_cursor(term_row, term_col);
}