#pragma once
#include <stdint.h>
#include "miniext.h"

// request type
#define IO_REQ_READ 1
#define IO_REQ_WRITE 2

// request status
#define IO_REQ_WAITING  1 // not handled yet
#define IO_REQ_BUSY     2 // is being handled
#define IO_REQ_FINISHED 3 // has finished being handled

// an asynchronous request to read/write a block of data to disk
typedef struct io_request {
    int type; // read/write
    int status;
    uint32_t block_num;
    // data points to BLOCK_SIZE bytes
    uint32_t * data;
    struct io_request * next;
    struct io_request * prev;
} io_request_t;
