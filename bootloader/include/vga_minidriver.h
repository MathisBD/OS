#pragma once

#include <stdint.h>

void vga_clear_screen();
void vga_print_char(char c);
void vga_print_string(const char* str);
void vga_print_hex_byte(uint8_t byte);
void vga_print_mem(const void* mem, uint32_t count);