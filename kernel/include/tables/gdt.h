#pragma once
#include <stdint.h>

void init_gdt(void);
void set_tss_esp(uint32_t addr);