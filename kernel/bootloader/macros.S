
.macro mInitSegments
    cli
    mov %dl, boot_drive // %dl contains the drive number
    mov $0, %ax 
    mov %ax, %ds 
    mov %ax, %es
    mov %ax, %ss 
    mov $BOOTLOADER_BASE, %sp 
    sti 
.endm

.macro mResetDisk
    mov boot_drive, %dl 
    xor %ax, %ax 
    int $0x13
    jc boot_fail // carry set == error
.endm

.macro mPrintString str 
    lea \str, %si 
    call PrintString 
.endm

.macro mReboot
    call Reboot
.endm
