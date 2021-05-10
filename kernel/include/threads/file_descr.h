#pragma once
#include <stdint.h>
#include <blocking_queue.h>
#include "interrupts/interrupts.h"
#include "drivers/dev.h"
#include "sync/queuelock.h"


// the folder used to access devices.
// e.g. the keyboard file is /dev/kbd.
#define DEV_FOLDER      "/dev"

// size of pipes. this should only make a difference
// for performance.
#define FD_PIPE_SIZE        4096

#define FD_TYPE_STREAM_DEV  1   // streaming device
#define FD_TYPE_FILE        2   // file on disk
#define FD_TYPE_PIPE        3   // pipe between two processes

#define FD_PERM_READ    0x1
#define FD_PERM_WRITE   0x2

typedef struct {
    queuelock_t* lock;
    uint32_t type;
    uint8_t perms; // permissions
    union {
        // FILE
        struct {
            uint32_t inode;
            uint32_t offset; // current file offset in bytes
        };
        // STREAMING DEVICE
        struct {
            stream_dev_t* dev;
        };
        // PIPE
        struct {
            blocking_queue_t* pipe;
        };
    };
} file_descr_t;

// open 
file_descr_t* kopen(char* name, uint8_t perms);
void kclose(file_descr_t*);
int kwrite(file_descr_t*, void* buf, uint32_t count);
int kread(file_descr_t*, void* buf, uint32_t count);
void kpipe(file_descr_t** from, file_descr_t** to);
//void kdup(file_descr_t* from, file_descr_t* to);


