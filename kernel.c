#include <stddef.h>
#include <stdint.h>

#if defined(__linux__)
    #error "This code must be compiled with a cross compiler"
#elif !defined(__i386__)
    #error "This code must be compiled with an x86-elf compiler"
#endif

// text output buffer
// entry format : BBBBFFFFCCCCCCCC
// - B : background color
// - F : foreground color
// - C : ASCII character
volatile uint16_t* vga_buffer = (uint16_t*)0xB8000;
const int VGA_COLS = 80;
const int VGA_ROWS = 80;
// black background, white foreground
const uint16_t TERM_COLOR = 0x0F00 ;

int term_col = 0;
int term_row = 0;


// clear the terminal
void term_clear()
{
    for (int col = 0; col < VGA_COLS; col++) {
        for (int row = 0; row < VGA_ROWS; row++) {
            const size_t index = (VGA_COLS * row + col);         
            vga_buffer[index] = TERM_COLOR | ' ';
        }
    }
}

void term_putc(char c)
{
    switch(c) {
    case '\n':
        {
            term_row++;
            term_col = 0;
            break;
        }
    default: 
        {
            const size_t index = (VGA_COLS * term_row) + term_col;
            vga_buffer[index] = TERM_COLOR | c;
            term_col++;
            break;
        }
    }

    if (term_col >= VGA_COLS) {
        term_col = 0;
        term_row++;
    }
    if (term_row >= VGA_ROWS) {
        term_col = 0;
        term_row = 0;
    }
}

void term_print(const char* str) 
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        term_putc(str[i]);
    }
}

void kernel_main()
{
    term_clear();
    
    term_print("hello world\nmy first string");
} 