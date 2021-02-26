#include "pit_driver.h"
#include <stdint.h>
#include "vga_driver.h"
#include "port_io.h"


// PIT io ports
#define PIT_DATA_0  0x40 // read/write
#define PIT_DATA_1  0x41 // unused
#define PIT_DATA_2  0x42 // unused
#define PIT_COMMAND 0x43 // write only

// Command flags

// channel : bits 7 and 6
#define CHANNEL_0       0x00

// access : bits 5 and 4
#define ACCESS_LOBYTE   0x10
#define ACCESS_HIGHBYTE 0x20
#define ACCESS_LOHI     0x30 // low byte then high byte (sequentially)

// operating mode : bits 3, 2 and 1
#define MODE_3          0x06 // square wave generator

// binary/BCD : bit 0
#define BINARY          0x00 

#define MAX_FREQ 1193181.6 // Hz (divider of 1)
#define MIN_FREQ (MAX_FREQ / 65535) // Hz


// returns the actual frequency
float init_pit(float requested_freq)
{
    // calculate the divider
    if (requested_freq < MIN_FREQ) {
        requested_freq = MIN_FREQ;
    }
    if (requested_freq > MAX_FREQ) {
        requested_freq = MAX_FREQ;
    }
    uint16_t divider = (uint16_t)(MAX_FREQ / requested_freq);
    // we must use an even divider
    divider &= 0xFFFE;
    
    // send the divider to the PIT
    port_int8_out(PIT_COMMAND, CHANNEL_0 | ACCESS_LOHI | MODE_3 | BINARY);

    port_int8_out(PIT_DATA_0, (divider & 0x00FF));
    port_int8_out(PIT_DATA_0, ((divider >> 8) & 0x00FF));

    // calculate the actual frequency
    return MAX_FREQ / (float)divider;
}