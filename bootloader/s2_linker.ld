
SECTIONS
{
    /* the .text of the assembly has to be at the very start */ 
    /* the output must consist of only a .text section 
       (otherwise we must do more work to extract and concat sections :
       ld there might be some zeroed out bytes between the sections we have
       to insert. it doesn't seem worth doing) */
    .text : AT(0x8000)
    {
        asm_code/second_stage.o(.text)
        *(.rodata)
        *(.text)
        *(.data)
        *(.bss)
    }
}