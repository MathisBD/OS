#pragma once
#include <stdint.h>


#define MMAP_ENT_SIZE 24

typedef struct {
    uint32_t mmap_ent_count;
    uint32_t mmap_addr;
} __attribute__((packed)) boot_info_t;
