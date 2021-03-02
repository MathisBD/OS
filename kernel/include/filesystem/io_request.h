#pragma once
#include <stdint.h>

// request type
#define IO_REQ_READ 1
#define IO_REQ_WRITE 2

// request status
#define IO_REQ_WAITING  1 // not handled yet
#define IO_REQ_FINISHED 2 // has finished being handled

// an asynchronous request to read/write a block of data to disk
typedef struct io_request {
    int type; // read/write
    int status;
    uint32_t block_num;
    uint32_t * data;
    uint32_t data_bytes; // number of bytes in data buffer
    struct io_request * next;
} io_request_t;