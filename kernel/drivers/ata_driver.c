#include "drivers/ata_driver.h"
#include "drivers/port_io.h"
#include <stdio.h>
#include <stdbool.h>
#include <panic.h>
#include "scheduler/timer.h"
#include <string.h>


#define SECTOR_SIZE 512

#define ATA_BSY       0x80 // busy
#define ATA_DRDY      0x40 // data ready
#define ATA_DF        0x20 
#define ATA_ERR       0x01 // error

#define CMD_READ  0x20
#define CMD_WRITE 0x30

// returns 0 if the operation was successful
int ata_wait(bool checkerr)
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
	
}

int rw_sector(uint32_t sector, void* data, bool write)
{
	if (ata_wait(true) != 0) {
		return -1;
	}
	
	port_int8_out(0x1F2, 1); // sector count
	port_int8_out(0x1F3, sector & 0xff);
	port_int8_out(0x1F4, (sector >> 8) & 0xff);
	port_int8_out(0x1F5, (sector >> 16) & 0xff); 
	port_int8_out(0x1F6, 0xE0 | ((sector >> 24) & 0xF));
	port_int8_out(0x1F7, write ? CMD_WRITE : CMD_READ);

	if (ata_wait(true) != 0) {
		return -1;
	}
	
	// WRITE
	if (write) {
		port_block_out(0x1F0, (uint32_t*)data, SECTOR_SIZE / sizeof(uint32_t));
	}
	// READ
	else {
		port_block_in(0x1F0, (uint32_t*)data, SECTOR_SIZE / sizeof(uint32_t));
	}
	return 0;
}

int ata_rw(uint32_t offset, uint32_t count, void* buf, bool write)
{
	uint32_t first_sct = offset / SECTOR_SIZE;
	uint32_t last_sct = (offset + count - 1) / SECTOR_SIZE;

	uint8_t* tmp_buf = malloc(SECTOR_SIZE);

	uint32_t buf_offs = 0;
	for (int i = first_sct; i <= last_sct; i++) {
		uint32_t start = i * SECTOR_SIZE;
		uint32_t ofs; 
		uint32_t cnt;

		if (start < offset) {
			ofs = offset - start;
			if (offset + count >= start + SECTOR_SIZE) {
				cnt = (start + SECTOR_SIZE) - offset;
			}	
			else {
				cnt = count;
			}
		}
		else {
			ofs = 0;
			if (offset + count >= start + SECTOR_SIZE) {
				cnt = SECTOR_SIZE;
			}
			else {
				cnt = (offset + count) - start;
			}
		}

		// WRITE
		if (write) {
			// we have to read the whole sector,
			// overwrite the part we want,
			// write back the whole sector
			if (rw_sector(i, tmp_buf, false) < 0) {
				free(tmp_buf);
				return -1;
			}
			memcpy(tmp_buf + ofs, buf + buf_offs, cnt);
			if (rw_sector(i, tmp_buf, true) < 0) {
				free(tmp_buf);
				return -1;
			}
		}
		// READ
		else {
			if (rw_sector(i, tmp_buf, false) < 0) {
				free(tmp_buf);
				return -1;
			}
			memcpy(buf + buf_offs, tmp_buf + ofs, cnt);
		}
		buf_offs += cnt;
	}
	free(tmp_buf);
	return 0;
}

int ata_read(uint32_t offset, uint32_t count, void* buf)
{
	ata_rw(offset, count, buf, false);
}

int ata_write(uint32_t offset, uint32_t count, void* buf)
{
	ata_rw(offset, count, buf, true);
}


// interrupt handler
// not used for now
void ata_primary_interrupt()
{
	//printf("ATA interrupt\n");
}
