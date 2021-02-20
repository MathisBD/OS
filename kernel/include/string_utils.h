#pragma once

#include <stdbool.h>
#include <stdint.h>

// reverses in place
void reverse_str(char * str);

// assumes num >= 0
// returns false if str is too small
bool int_to_string_base(uint64_t num, char * str, int str_size, int base);
// base 10
bool int_to_string(uint64_t num, char * str, int str_size);