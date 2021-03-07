#include <stdint.h>


int ata_read_sector(uint32_t sector, uint8_t* data);
int ata_write_sector(uint32_t sector, uint8_t* data);