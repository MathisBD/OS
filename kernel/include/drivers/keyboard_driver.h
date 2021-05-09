#pragma once
#include <stdbool.h>

typedef struct {
    bool ctrl_pressed;
    bool alt_pressed;
    bool lshift_pressed;
    bool rshift_pressed;
} kbd_state_t;

void init_keyboard_driver(void);
void keyboard_interrupt(void);