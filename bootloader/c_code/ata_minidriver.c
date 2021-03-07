#include <stdbool.h>
#include "ata_minidriver.h"
#include "pio.h"
#include "vga_minidriver.h"

#define SECTOR_SIZE 512

#define ATA_BSY       0x80 // busy
#define ATA_DRDY      0x40 // data ready
#define ATA_DF        0x20 
#define ATA_ERR       0x01 // error

#define CMD_READ  0x20
#define CMD_WRITE 0x30
#define CMD_RDMUL 0xc4
#define CMD_WRMUL 0xc5

// returns 0 if the operation was successful
bool ata_wait(bool checkerr)
{
	int r;
	while(((r = port_int8_in(0x1f7)) & (ATA_BSY | ATA_DRDY)) != ATA_DRDY) {
	}

	if(checkerr && (r & (ATA_DF | ATA_ERR)) != 0) {
		return -1;
	}
	return 0;
}

void send_data(uint32_t sector, uint8_t sector_count)
{
	port_int8_out(0x1F2, sector_count);
	port_int8_out(0x1F3, sector & 0xff);
	port_int8_out(0x1F4, (sector >> 8) & 0xff);
	port_int8_out(0x1F5, (sector >> 16) & 0xff); 
	port_int8_out(0x1F6, 0xE0 | ((sector >> 24) & 0xF));
}

int ata_read_sector(uint32_t sector, uint8_t* data)
{
	if (ata_wait(true) != 0) {
		return -1;
	}
	
	send_data(sector, 1);
	port_int8_out(0x1F7, CMD_READ);

	if (ata_wait(true) != 0) {
		return -1;
	}
	
	port_block_in(0x1F0, (uint32_t*)data, SECTOR_SIZE / sizeof(uint32_t));
	return 0;
}

int ata_write_sector(uint32_t sector, uint8_t* data)
{
	if (ata_wait(true) != 0) {
		return -1;
	}
	
	send_data(sector, 1);
	port_int8_out(0x1F7, CMD_WRITE);

	if (ata_wait(true) != 0) {
		return -1;
	}

	port_block_out(0x1F0, (uint32_t*)data, SECTOR_SIZE / sizeof(uint32_t));
	return 0;
}