#pragma once

#include <stdbool.h>
#include <stdint.h>

// loads a program in elf format from a file on disk.
void load_program(
    char* prog_name, 
    uint32_t* p_entry_addr, 
    uint32_t* p_user_stack_top,
    uint32_t* p_data_size,
    uint32_t* p_stack_size);
