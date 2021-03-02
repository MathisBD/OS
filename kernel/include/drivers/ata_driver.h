#pragma once
#include "filesystem/io_request.h"


void ata_pio_request(io_request_t * request);
void ata_primary_interrupt();