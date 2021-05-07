#pragma once

#include <stdbool.h>
#include <stdint.h>

// loads a program in elf format from a file on disk.
void load_program(char* prog_name, uint32_t* entry_addr, uint32_t* user_stack_top);
