#pragma once
#include <stdint.h>


#define MMAP_ENT_SIZE 24

// info the bootloader passes to the kernel
typedef struct {
    uint32_t mmap_ent_count;
    uint32_t mmap_addr;
} __attribute__((packed)) boot_info_t;


#define MMAP_ENT_TYPE_AVAILABLE 1
#define MMAP_ENT_TYPE_RESERVED  2
#define MMAP_ENT_TYPE_ACPI_RECLAIMABLE 3
#define MMAP_ENT_TYPE_ACPI_NVS  4
#define MMAP_ENT_TYPE_BAD_MEM   5    

typedef struct {
    uint64_t base;
    uint64_t length;
    uint32_t type;
    uint32_t unused;
} __attribute__((packed)) mmap_entry_t;