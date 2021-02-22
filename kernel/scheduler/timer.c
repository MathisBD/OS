#include "timer.h"
#include <stdint.h>
#include "string_utils.h"

float freq; // frequency of interrupts, in Hz
float delta_t; // time between interrupts, in seconds

float time; // in seconds


void init_timer(float f)
{
    freq = f;
    delta_t = 1.0 / f;

    time = 0.0;
}

void timer_interrupt()
{
    if ((uint32_t)(time + delta_t) > (uint32_t)time) {
        //vga_print_int(time + delta_t, 10);
        //vga_print("\n");
    }
    time += delta_t;
}
