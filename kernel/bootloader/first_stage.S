.code16  // we are in 16-bit real mode

// this is the address the bootloader is loaded
// we have to trim the first 0x7c00 bytes to get the 
// boot sector and copy it to the first disk sector
.org 0x7C00
.text

.include "bootloader/macros.S"

bootloader_start: 
    jmp load 

// BOOT INFO to load the kernel
// this is filled in when the disk image is created
// it has to be at offset 1byte in the bootloader
kblock_num: .word 0
kblock_count: .word 0


// INSTRUCTIONS
load:
    mInitSegments
    mResetDisk 
    mPrintString loading_msg
    

halt:
    cli
    hlt

boot_fail:
    mReboot


// FUNCTIONS
.include "bootloader/functions.S"


// DATA (after the instructions of course)
loading_msg: .asciz "Loading kernel...\n\r"
reboot_msg: .asciz "ERROR - Press any key to reboot\n\r"
boot_drive: .byte 0

bootloader_end:
// pad up to 510 bytes (leave space for the boot magic number)
.fill (510 - (bootloader_end - bootloader_start)), 1, 0
boot_magic: .int 0xAA55 // for the bios to recognize us

