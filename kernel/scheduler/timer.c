#include "timer.h"
#include <stdint.h>
#include "string_utils.h"
#include "vga_driver.h"

float freq; // frequency of interrupts, in Hz
float delta_t; // time between interrupts, in seconds

float time; // in seconds


void init_timer(float f)
{
    freq = f;
    delta_t = 1.0 / f;

    time = 0.0;
}

void timer_interrupt(uint32_t tmp)
{
    if ((uint32_t)(1*(time + delta_t)) > (uint32_t)(1*time)) {
        vga_print_int((uint32_t)(time + delta_t), 10);
        vga_print(" ---> ");
        vga_print_int(tmp, 16);
        vga_print("\n");
    }
    time += delta_t;
}
