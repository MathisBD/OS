#include <stddef.h>
#include "drivers/vga_driver.h"
#include "memory/constants.h"
#include "drivers/port_io.h"
#include "drivers/dev.h"
#include "memory/kheap.h"
#include "filesystem/file_descr.h"
#include <panic.h>
#include <string.h>
#include <stdio.h>

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


// layout of the vfa file :
// first 4 bytes : header
// next 2*ROWS*COLS bytes :
// (color byte + char byte) for each cell of each row 
struct {
    uint8_t rows;
    uint8_t cols;
    uint8_t term_row;
    uint8_t term_col;
} file_header;
static uint32_t file_ofs;

static queuelock_t* vga_lock;

// text output buffer
// entry format : 0xBFCC
// - B : background color
// - F : foreground color
// - C : ASCII character
static uint16_t* vga_buffer = V_VGA_ADDRESS;

static uint32_t term_col = 0;
static uint32_t term_row = 0;
// white text, black background
static uint8_t default_color = 0x0F;

static uint16_t vga_entry(uint8_t color, char c)
{
    return (((uint16_t)color) << 8) | c;
}

static void set_vga_buffer(int row, int col, uint8_t color, char c)
{
    const int index = VGA_COLS * row + col;
    vga_buffer[index] = vga_entry(color, c);
}

static void clear_screen(void)
{
    for (int col = 0; col < VGA_COLS; col++) {
        for (int row = 0; row < VGA_ROWS; row++) {      
            set_vga_buffer(row, col, default_color, ' ');
        }
    }
}

static void enable_cursor(void)
{
    uint8_t b;

    port_int8_out(CURSOR_PORT_1, 0x0A);
    b = port_int8_in(CURSOR_PORT_2);
    port_int8_out(CURSOR_PORT_2, (b & 0xC0) | CURSOR_TOP);

    port_int8_out(CURSOR_PORT_1, 0x0B);
    b = port_int8_in(CURSOR_PORT_2);
    port_int8_out(CURSOR_PORT_2, (b & 0xE0) | CURSOR_BOTTOM);
}

static void disable_cursor(void)
{
    port_int8_out(CURSOR_PORT_1, 0x0A);
    port_int8_out(CURSOR_PORT_2, 0x20);
}

static void move_cursor(int row, int col)
{
    uint16_t pos = row * VGA_COLS + col;

    port_int8_out(CURSOR_PORT_1, 0x0F);
    port_int8_out(CURSOR_PORT_2, (uint8_t)(pos & 0xFF));

    port_int8_out(CURSOR_PORT_1, 0x0E);
    port_int8_out(CURSOR_PORT_2, (uint8_t)((pos & 0xFF00) >> 8));
}

void start_init_vga_driver()
{
    clear_screen();

    // setup cursor
    enable_cursor();
    move_cursor(0, 0);
}



static void private_putchar(char c) 
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

void vga_print_mem(void* addr, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        vga_print_
    }
}

void vga_print(const char* str) 
{
    for (size_t i = 0; str[i] != '\0'; i++) {
        private_putchar(str[i]);
    }
    move_cursor(term_row, term_col);
}

static uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

// write to vga_buffer file
static int vga_write_buffer(uint8_t* buf, uint32_t count)
{
    printf("vga write buffer : file_ofs = %u\n", file_ofs);
    kql_acquire(vga_lock);
    uint32_t i = 0;
    if (file_ofs == 0 && count > 0) {
        i++; file_ofs++; count--;
    }
    if (file_ofs == 1 && count > 0) {
        i++; file_ofs++; count--;
    }
    if (file_ofs == 2 && count > 0) {
        term_row = buf[i]; i++; file_ofs++; count--;
        move_cursor(term_row, term_col);
    }
    if (file_ofs == 3 && count > 0) {
        term_col = buf[i]; i++; file_ofs++; count--;
        move_cursor(term_row, term_col);
    }
    count = min(count, 2*VGA_COLS*VGA_ROWS - (file_ofs-4));
    memcpy(((void*)vga_buffer) + (file_ofs-4), buf + i, count);
    file_ofs += count;
    kql_release(vga_lock);
    return 0;
}

static int vga_read_buffer(uint8_t* buf, uint32_t count)
{
    kql_acquire(vga_lock);
    uint32_t i = 0;
    if (file_ofs == 0 && count > 0) {
        buf[i] = VGA_ROWS; i++; file_ofs++; count--;
    }
    if (file_ofs == 1 && count > 0) {
        buf[i] = VGA_COLS; i++; file_ofs++; count--;
    }
    if (file_ofs == 2 && count > 0) {
        buf[i] = term_row; i++; file_ofs++; count--;
    }
    if (file_ofs == 3 && count > 0) {
        buf[i] = term_col; i++; file_ofs++; count--;
    }
    count = min(count, 2*VGA_COLS*VGA_ROWS - (file_ofs-4));
    memcpy(buf + i, ((void*)vga_buffer) + (file_ofs-4), count);
    file_ofs += count;
    kql_release(vga_lock);
    return 0;
}

// seek in the vga_buffer file
static void vga_seek_buffer(int ofs, uint8_t flags)
{
    kql_acquire(vga_lock);
    switch(flags)
    {
    case FD_SEEK_CUR: file_ofs += ofs; break;
    case FD_SEEK_SET: file_ofs = ofs; break;
    case FD_SEEK_END: file_ofs = 4 + 2*VGA_ROWS*VGA_COLS + ofs; break;
    default: vga_print("vga_seek() : unknown seek flag");
    }
    kql_release(vga_lock);
}

// write a string at the cursor position
static int vga_write(void* buf, uint32_t count)
{
    kql_acquire(vga_lock);
    
    for (int i = 0; i < count; i++) {
        private_putchar(((char*)buf)[i]);
    }
    move_cursor(term_row, term_col);
    
    kql_release(vga_lock);
    return count;
}

void finish_init_vga_driver()
{
    vga_lock = kql_create();
    file_ofs = 0;

    // device for writing to the vga file
    stream_dev_t* dev = kmalloc(sizeof(stream_dev_t));
    dev->lock = kql_create();
    dev->name = "vga_buffer";
    dev->perms = FD_PERM_WRITE | FD_PERM_READ | FD_PERM_SEEK;
    dev->write = vga_write_buffer;
    dev->read = vga_read_buffer;
    dev->seek = vga_seek_buffer;
    register_stream_dev(dev);

    // device for writting characters at the cursor position
    dev = kmalloc(sizeof(stream_dev_t));
    dev->lock = kql_create();
    dev->name = "vga";
    dev->perms = FD_PERM_WRITE;
    dev->write = vga_write;
    register_stream_dev(dev);
}