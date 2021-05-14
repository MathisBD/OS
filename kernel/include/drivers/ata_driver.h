#pragma once
#include <stdint.h>

void init_ata_driver();
int ata_read(uint32_t offset, uint32_t count, void* buf);
// we can only write whole sectors
int ata_write_sector(uint32_t sector, void* buf);
//void ata_primary_interrupt();