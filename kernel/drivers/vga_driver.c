#include <stddef.h>
#include "drivers/vga_driver.h"
#include "memory/constants.h"
#include "drivers/port_io.h"


// io ports to communicate with the vga cursor
#define CURSOR_PORT_1 0x3D4
#define CURSOR_PORT_2 0x3D5

#define VGA_COLS 80
#define VGA_ROWS 25

// sets the height of the cursor
// range : 0 (very top) to 15 (very bottom)
#define CURSOR_TOP 13
#define CURSOR_BOTTOM 15

#define TAB_ALIGN 8


// text output buffer
// entry format : 0xBFCC
// - B : background color
// - F : foreground color
// - C : ASCII character
uint16_t* vga_buffer;

uint32_t term_col;
uint32_t term_row;
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

    port_int8_out(CURSOR_PORT_1, 0x0A);
    b = port_int8_in(CURSOR_PORT_2);
    port_int8_out(CURSOR_PORT_2, (b & 0xC0) | CURSOR_TOP);

    port_int8_out(CURSOR_PORT_1, 0x0B);
    b = port_int8_in(CURSOR_PORT_2);
    port_int8_out(CURSOR_PORT_2, (b & 0xE0) | CURSOR_BOTTOM);
}

void disable_cursor(void)
{
    port_int8_out(CURSOR_PORT_1, 0x0A);
    port_int8_out(CURSOR_PORT_2, 0x20);
}

void move_cursor(int row, int col)
{
    uint16_t pos = row * VGA_COLS + col;

    port_int8_out(CURSOR_PORT_1, 0x0F);
    port_int8_out(CURSOR_PORT_2, (uint8_t)(pos & 0xFF));

    port_int8_out(CURSOR_PORT_1, 0x0E);
    port_int8_out(CURSOR_PORT_2, (uint8_t)((pos & 0xFF00) >> 8));
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

void private_putchar(char c) 
{
   if (term_row >= VGA_ROWS) {
        term_col = 0;
        term_row = 0;
        clear_screen();
    }
    
    switch(c) {
    case '\n':
    case '\r':
        term_row++;
        term_col = 0;
        break;
    case '\t':
        term_col &= ~(TAB_ALIGN-1);
        term_col += TAB_ALIGN;
        break;
    default:     
        set_vga_buffer(term_row, term_col, default_color, c);
        term_col++;
        break;
    }

    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
}

void vga_putchar(char c)
{
    private_putchar(c);
    move_cursor(term_row, term_col);
}

void vga_print(const char* str) 
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        private_putchar(str[i]);
    }
    move_cursor(term_row, term_col);
}