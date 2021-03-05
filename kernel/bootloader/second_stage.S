.code16  // we are in 16-bit real mode
// adress we are loaded at by the first stage bootloader
.org 0x10000
.text 

// size of the second stage file 
// this way it always fits in the direct blocks of its inode
.set SECOND_STAGE_SIZE, 12*1024 

s2_code_start:
    mov $0x0E42, %ax
    mov $9, %bx 
    int $0x10

halt:
    cli 
    hlt

s2_code_end:
.fill (SECOND_STAGE_SIZE - (s1_code_end - s1_code_start)), 1, 0