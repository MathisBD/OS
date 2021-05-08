#pragma once
#include <stdint.h>
#include <blocking_queue.h>
#include "interrupts/interrupts.h"


#define FD_TYPE_IO      1
#define FD_TYPE_FILE    2
#define FD_TYPE_PIPE    3

#define FD_PERM_READ    1
#define FD_PERM_WRITE   2
#define FD_PERM_RW      3

typedef struct {
    uint32_t type;
    uint32_t perms; // permissions
    blocking_queue_t* buf;
} file_descr_t;


file_descr_t* kopen(char* name);
void kclose(file_descr_t*);
int kwrite(file_descr_t*, void* buf, uint32_t count);
int kread(file_descr_t*, void* buf, uint32_t count);
void kpipe(file_descr_t* from, file_descr_t* to);
void kdup(file_descr_t* from, file_descr_t* to);


