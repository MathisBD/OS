#pragma once
#include <stdint.h>
#include <stdbool.h>

#define KBD_FLAG_CTRL   0x1
#define KBD_FLAG_ALT    0x2
#define KBD_FLAG_LSHIFT 0x4
#define KBD_FLAG_RSHIFT 0x8

typedef struct {
    char* c;
    uint8_t flags;
} key_t;

void init_kbd_driver();
void kbd_interrupt();
