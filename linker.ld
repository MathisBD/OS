/* The bootloader will hand us execution at ENTRY */ 
ENTRY(start)

SECTIONS
{
    /* Beggining of the kernel. Space below 1MB is reserved for x86 stuff. */
    . = 1M;

    /* Multiboot header near the start, otherwise the bootloader won't find it. */
    .text BLOCK(4K) : ALIGN(4K)
    {
        *(.multiboot)
        *(.text)
    }

    .rodata BLOCK(4K) : ALIGN(4K)
    {
        *(.rodata)
    }

    .data BLOCK(4K) : ALIGN(4K)
    {
        *(.data)
    }

    /* read-write data (uninitialized) and stack */
    .bss BLOCK(4K) : ALIGN(4K)
    {
        *(COMMON)
        *(.bss)
    }
}