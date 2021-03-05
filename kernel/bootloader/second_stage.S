.code16  // we are in 16-bit real mode
// adress we are loaded at by the first stage bootloader
.org 0x8000
.text 

// size of the second stage file 
// this way it always fits in the direct blocks of its inode
.set SECOND_STAGE_SIZE, 12*1024 

.include "bootloader/macros.S"

s2_code_start:
    lea msg, %si 
    call PrintString

halt:
    cli 
    hlt

// DATA 
msg: .asciz "Second stage yay !!!"

.include "bootloader/functions.S"

s2_code_end:
.fill (SECOND_STAGE_SIZE - (s2_code_end - s2_code_start)), 1, 0