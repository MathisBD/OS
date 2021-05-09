#pragma once
#include <blocking_queue.h>
#include <stdint.h>
#include "sync/queuelock.h"

#define STREAM_DEV_BUF_SIZE     1024

// does the device support reading (e.g. keyboard) ?
#define DEV_FLAG_READ   0x1
// does the device support writting (e.g. vga display) ?
#define DEV_FLAG_WRITE  0x2


// a device where we can only write (and/or read) a stream of bytes
typedef struct {
    queuelock_t* lock;
    char* name;
    uint8_t flags;
    // buffer used to read from the device
    blocking_queue_t* read_buf;
    // buffer used to write to the device
    blocking_queue_t* write_buf;
} stream_dev_t;

// a request to read/write a block
typedef struct _block_req {
    queuelock_t* lock;
    bool read; // otherwise write
    void* buf;
    uint32_t block_num;
    struct _block_req* next; // next request to process
} block_req_t;

// a device where we can write/read blocks at different locations
// (e.g. a hard-drive).
typedef struct {
    queuelock_t* lock;
    char* name;
    uint8_t flags;
    uint32_t block_size;
    block_req_t* first_req;
    block_req_t* last_req;
} block_dev_t;


stream_dev_t* register_stream_dev(char* name, uint8_t flags);
block_dev_t* register_block_dev(char* name, uint8_t flags);

stream_dev_t* get_stream_dev(char* name);
block_dev_t* get_block_dev(char* name);

void unregister_stream_dev(stream_dev_t* dev);
void unregister_block_dev(block_dev_t* dev);