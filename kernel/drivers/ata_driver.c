#include "ata_driver.h"
#include "port_io.h"
#include <stdio.h>
#include <stdbool.h>
#include "panic.h"

#define SECTOR_SIZE 512

#define ATA_BSY       0x80
#define ATA_DRDY      0x40
#define ATA_DF        0x20
#define ATA_ERR       0x01

#define CMD_READ  0x20
#define CMD_WRITE 0x30
#define CMD_RDMUL 0xc4
#define CMD_WRMUL 0xc5

io_request_t* first_request = 0;
io_request_t* last_request = 0;


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

void ata_pio_request(io_request_t* request) 
{
	if (last_request) {
		last_request->next = request;
		request->prev = last_request;
		request->next = 0;
		last_request = request;
	}
	else {
		first_request = request;
		last_request = request;
		request->prev = 0;
		request->next = 0;
		handle_request();
	}
}

// handle the first request
void handle_request()
{	
	first_request->status = IO_REQ_BUSY;

	uint8_t sector_count = BLOCK_SIZE / SECTOR_SIZE;
	uint32_t sector = first_request->block_num;
	if (sector_count == 0) {
		panic("handle_request : no sector");
	}	
	if (sector_count >= 8) {
		panic("handle_request : too many sectors");
	}

	ata_wait(false);
	port_int8_out(0x1F2, sector_count);
	port_int8_out(0x1F3, sector & 0xff);
	port_int8_out(0x1F4, (sector >> 8) & 0xff);
	port_int8_out(0x1F5, (sector >> 16) & 0xff); 
	port_int8_out(0x1F6, 0xE0 | ((sector >> 24) & 0xF));

	// send the r/w command
	if (first_request->type == IO_REQ_READ) {
		if (sector_count == 1) {
			port_int8_out(0x1F7, CMD_READ); 
		}
		else {
			port_int8_out(0x1F7, CMD_RDMUL);
		}	
	}
	else if (first_request->type == IO_REQ_WRITE) {
		if (sector_count == 1) {
			port_int8_out(0x1F7, CMD_WRITE);
		}
		else {
			port_int8_out(0x1F7, CMD_WRMUL);
		}
		ata_wait(false);
		// divide by 4 because of int <--> byte
		port_block_out(0x1F0, first_request->data, BLOCK_SIZE / sizeof(uint32_t));
		port_int8_out(0x1F7, 0xE7); // flush
	}
	else {
		panic("handle_request : invalid request type");
	}
}

// interrupt handler
void ata_primary_interrupt()
{
	printf("ATA interrupt\n");


	if (ata_wait(true) >= 0 && first_request->type == IO_REQ_READ) {
		port_block_in(0x1F0, first_request->data, BLOCK_SIZE / sizeof(uint32_t));
	}

	first_request->status = IO_REQ_FINISHED;

	if (first_request == last_request) {
		first_request = 0;
		last_request = 0;
	}
	else {
		// remove the first request
		io_request_t * old = first_request;
		first_request = first_request->next;
		old->next = 0;
		first_request->prev = 0;
		// handle the next one
		handle_request();
	}
}
