#include "drivers/timer_driver.h"
#include <stdint.h>
#include "threads/scheduler.h"


float freq; // frequency of interrupts, in Hz
float delta_time; // time between interrupts, in milliseconds
float time; // in milliseconds


void init_timer(float f)
{
    freq = f;
    delta_time = 1000.0 / f;
    time = 0.0;
}

void timer_interrupt()
{
    time += delta_time;
    timer_tick(delta_time);
}

void wait(float t)
{
    float end = time + t;
    while (time < end) {
    }
    return;
}