#pragma once

#include <stdint.h>


// called very early in the initialization phase, 
// so that other programs can call vga_print()
// (not thread safe).
void start_init_vga_driver();
// called once threads/processes are enabled,
// so that other programs can access the vga driver
// through the streaming device interface.
void finish_init_vga_driver();

// these functions aren't thread safe. 
// thread safe printing : write to /dev/vga.
void vga_putchar(char c);
void vga_print(const char* str);
void vga_print_int(uint64_t num, int base);