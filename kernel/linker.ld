#include "memory/constants.h"

/* The bootloader will hand us execution here */
/* By convention, for the kernel executable, this is
   the PHYSICAL address of the entry point */ 
ENTRY(phys_entry)

SECTIONS
{
    /* Beggining of the kernel. Space below 1MB is reserved for x86 stuff. */
    /* The kernel will live at 3GB + 1MB = 0xC0000000 + 1MB in virtual memory */
    . = V_AVAILABLE_START;

    .bss ALIGN(4K) : AT(ADDR(.bss) - V_KERNEL_START)
    {
        KEEP(*(COMMON))
        KEEP(*(.bss*))
        . = ALIGN(4K);
    }
    .text ALIGN(4K) : AT(ADDR(.text) - V_KERNEL_START)
    {
        _kernel_start = .;
        KEEP(*(.text*)) 
        . = ALIGN(4K);
    } 
    .rodata ALIGN(4K) : AT(ADDR(.rodata) - V_KERNEL_START)
    {
        KEEP(*(.rodata*))
        . = ALIGN(4K);
        _kernel_ro_end = .;
    }
 
    .data ALIGN(4K) : AT(ADDR(.data) - V_KERNEL_START)
    {
        KEEP(*(.data*))
        . = ALIGN(4K);
    }
    _kernel_rw_end = .;
}