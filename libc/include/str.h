#pragma once
#include <stdint.h>

// resizable strings
typedef struct {
    char* buf;  // contains the characters + the null terminator
    uint32_t size;     // number of characters in the string + 1 (for the null terminator)
    uint32_t capacity; // size of buf
} str_t;


str_t* str_create();
void str_delete(str_t* str);
void str_grow(str_t* str, uint32_t cap);
void str_add_char(str_t* str, char c);
void str_add_cstr(str_t* str, const char* cstr);
// returns a null terminated string
char* str_get_cstr(str_t* str);
