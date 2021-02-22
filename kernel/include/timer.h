#pragma once


// called by the interrupt handler on each IRQ 0
void timer_interrupt();

// freq is the frequency at which timer_interrupt()
// is called, in Hz
void init_timer(float freq);