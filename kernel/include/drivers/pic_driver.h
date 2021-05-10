#pragma once
#include <stdint.h>


void init_pic_driver();
void pic_eoi(uint32_t irq);
void enable_irq(uint32_t irq);
void disable_irq(uint32_t irq);