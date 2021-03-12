#pragma once
#include <stdint.h>


int ata_read(uint32_t offset, uint32_t count, void* buf);
int ata_write(uint32_t offset, uint32_t count, void* buf);
void ata_primary_interrupt();