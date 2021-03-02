#pragma once

#include <stdint.h>

extern void port_int8_out(uint16_t io_port, uint8_t data);
extern void port_int16_out(uint16_t io_port, uint16_t data);
extern void port_int32_out(uint16_t io_port, uint32_t data);
extern void port_block_out(uint16_t io_port, uint32_t * src, uint32_t count);

extern uint8_t port_int8_in(uint16_t io_port);
extern void port_block_in(uint16_t io_port, uint32_t * dest, uint32_t count);