#pragma once

#include <stdbool.h>

// loads a program in elf format from a file on disk.
// sets up a new kernel stack for the program,
// and returns the address of this stack.
void* load_program(char* prog_name);
