.code16  // we are in 16-bit real mode
//.org 0x0 // first stage bootloader will be loaded at the start of ram
//.text

start:
    mov $0x42, %al
    mov $0xE, %ah
    int $0x10
halt:
    cli
    hlt

msg:
    .asciz "hello world"

end:
// pad up to 510 bytes (leave space for the boot magic number)
.fill (0x1fe-(end-start)), 1, 0
boot_magic: .int 0xAA55 // for the bios to recognize us
