#include "timer.h"
#include <stdint.h>
#include "string_utils.h"
#include "vga_driver.h"

float freq; // frequency of interrupts, in Hz
float delta_time; // time between interrupts, in seconds

float time; // in seconds


void init_timer(float f)
{
    freq = f;
    delta_time = 1.0 / f;

    time = 0.0;
}

void timer_interrupt(uint32_t tmp)
{
    if ((uint32_t)(1*(time + delta_time)) > (uint32_t)(1*time)) {
        //printf("%d ---> %x\n", (uint32_t)(time + delta_time), tmp);
    }
    time += delta_time;
}

void wait(float t)
{
    float end = time + t;
    while (time < end) {
    }
    return;
}