ENTRY(main)

SECTIONS
{
    .text ALIGN(4K) : {
        *(.text*)
    }
    .data ALIGN(4K) : {
        *(.data*)
    }
    .rodata ALIGN(4K) : {
        *(.rodata*)
    }
    .bss ALIGN(4K) : {
        *(.bss*)
    }
}