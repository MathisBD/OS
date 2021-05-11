#pragma once
#include <blocking_queue.h>
#include <stdint.h>
#include "sync/queuelock.h"


// a device where we can read/write bytes
typedef struct {
    queuelock_t* lock;
    char* name;
    uint8_t perms;  // FD perms
    // return the actual number of bytes read/written
    int (*read)(void* buf, uint32_t count);
    int (*write)(void* buf, uint32_t count);
    void (*seek)(int ofs, uint8_t flags); // FD seek() flags
} stream_dev_t;

// a request to read/write a block
typedef struct _block_req {
    queuelock_t* lock;
    bool read; // otherwise write
    void* buf;
    uint32_t block_num;
    struct _block_req* next; // next request to process
} block_req_t;

// a device where we can write/read blocks
// (e.g. a hard-drive).
typedef struct {
    queuelock_t* lock;
    char* name;
    uint8_t perms;
    uint32_t block_size;
    block_req_t* first_req;
    block_req_t* last_req;
} block_dev_t;


// call this before initializing other drivers
void init_dev();

void register_stream_dev(stream_dev_t* dev);
void register_block_dev(block_dev_t* dev);

stream_dev_t* get_stream_dev(char* name);
block_dev_t* get_block_dev(char* name);

void unregister_stream_dev(stream_dev_t* dev);
void unregister_block_dev(block_dev_t* dev);
