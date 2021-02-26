#pragma once

#include <stdint.h>


void init_vga_driver(void);
void vga_putchar(char c);
void vga_print(const char* str);
void vga_print_int(uint64_t num, int base);