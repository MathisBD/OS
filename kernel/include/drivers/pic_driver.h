#pragma once
#include <stdint.h>

#define TIMER_IRQ		0
#define KEYBOARD_IRQ	1
#define ATA_IRQ			14

void init_pic_driver();
void pic_eoi(uint32_t irq);
void enable_irq(uint32_t irq);
void disable_irq(uint32_t irq);