

.macro mInitSegments
    cli
    mov $0, %ax 
    mov %ax, %ds 
    mov %ax, %es
    mov %ax, %ss 
    mov $0x7C00, %sp 
    sti 
.endm

.macro mResetDisk

.endm