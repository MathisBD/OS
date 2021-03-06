
// expects the address of a data buffer in ds:si 
// and a count in cx
.func PrintMem
PrintMem:
    mov $0, %dx // number of bytes printed 
PrintMem_loop:
    cmp %cx, %dx 
    jge PrintMem_return 

    cld
    lodsb 
    inc %dx
    
    push %ax 

    // high (hex) digit
    sar $4, %al
    and $0xF, %al
    mPrintHexDigit

    // low (hex) digit
    pop %ax
    and $0xF, %ax
    mPrintHexDigit

    // space 
    mov $0x0E20, %ax
    mov $9, %bx 
    int $0x10

    // newline (every 16 bytes)
    test $15, %dx 
    jnz PrintMem_loop
    // line feed
    mov $0x0E0A, %ax
    mov $9, %bx 
    int $0x10
    // carriage return 
    mov $0x0E0D, %ax
    mov $9, %bx 
    int $0x10
    jmp PrintMem_loop
PrintMem_return:
    ret
.endfunc
