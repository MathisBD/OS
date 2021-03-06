
// We write a byte at ffff:0510.
// If the A20 line is disabled,
// this should write at 0000:0500
// (we use 0x500 be cause this is the end 
// of BIOS reserved memory).
// Returns 1 if the line is enabled,
// and 0 if it is disabled
.func CheckA20
CheckA20:
    push %ax 
    push %ds 
    push %es 
    push %si 
    push %di 
    cli

    // es:di=0000:0500
    mov $0x0000, %ax
    mov %ax, %es 
    mov $0x0500, %di

    // ds:si=ffff:0510
    mov $0xffff, %ax 
    mov %ax, %ds 
    mov $0x510, %si

    // save byte at es:di
    mov %es:(%di), %al 
    push %ax 

    // save byte at ds:si 
    mov %ds:(%si), %al 
    push %ax 

    movb $0x00, %es:(%di)
    movb $0xff, %ds:(%si)

    cmpb $0xff, %es:(%di)
    je CheckA20_disabled

CheckA20_enabled:
    mov $1, %ax 
    jmp CheckA20_cleanup

CheckA20_disabled:
    mov $0, %ax 

CheckA20_cleanup:
    // restore byte at ds:si 
    pop %ax 
    mov %al, %ds:(%si)

    // restore byte at es:di 
    pop %ax 
    mov %al, %es:(%di)

    sti
    pop %di 
    pop %si 
    pop %es 
    pop %ds 
    pop %ax 
    ret 
.endfunc

.macro mSetA20Bios
    mov $0x2401, %ax 
    int $0x15
.endm 

.macro mSetA20FastGate 
    in $0x92, %al 
    or $2, %al 
    out %al, $0x92
.endm 

.macro mEnableA20
    call CheckA20
    test %ax, %ax 
    jnz EnableA20_done

    mSetA20Bios 
    call CheckA20
    test %ax, %ax 
    jnz EnableA20_done

    mSetA20FastGate
    call CheckA20
    test %ax, %ax 
    jnz EnableA20_done
EnableA20_fail:
    mPrintString a20_error_msg 
    mReboot
EnableA20_done:
.endm
