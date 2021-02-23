ENTRY(_main)

SECTIONS
{
    .text ALIGN(4K) : {
        *(.text)
    }
    .data ALIGN(4K) : {
        *(.data)
    }
    .bss ALIGN(4K) : {
        *(.bss)
    }
}