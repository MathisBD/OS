#pragma once
#include <stdbool.h>
#include "init/bootloader_info.h"

// information about kernel initialization
bool is_idt_init();
bool is_gdt_init();
bool is_drivers_init();
bool is_paging_init();
bool is_kheap_init();
bool is_fs_init();
bool is_threads_init();
bool is_process_init();
bool is_all_init();

void init_kernel(boot_info_t* boot_info);