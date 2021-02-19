#include "constants.h"

/* The bootloader will hand us execution at ENTRY */ 
ENTRY(_start)

SECTIONS
{
    /* Beggining of the kernel. Space below 1MB is reserved for x86 stuff. */
    /* The kernel will live at 3GB + 1MB = 0xC0000000 + 1MB in virtual memory */
    . = V_AVAILABLE_START;

    /* Multiboot header near the start, otherwise the bootloader won't find it. */
    .text ALIGN(PAGE_SIZE) : AT(ADDR(.text) - V_KERNEL_START)
    {
        _kernel_start = .;
        *(.text) 
        . = ALIGN(PAGE_SIZE);
    } 
    .rodata ALIGN(PAGE_SIZE) : AT(ADDR(.rodata) - V_KERNEL_START)
    {
        *(.rodata)
        . = ALIGN(PAGE_SIZE);
        _kernel_ro_end = .;
    }
 
    .data ALIGN(PAGE_SIZE) : AT(ADDR(.data) - V_KERNEL_START)
    {
        *(.data)
        . = ALIGN(PAGE_SIZE);
    }
    .bss ALIGN(PAGE_SIZE) : AT(ADDR(.bss) - V_KERNEL_START)
    {
        *(COMMON)
        *(.bss)
        . = ALIGN(PAGE_SIZE);
    }
    _kernel_rw_end = .;
}