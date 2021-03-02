
// expects a string in ds:si
.func PrintString
PrintString:
    lodsb
    or %al, %al 
    jz PrintString_return

    mov $0xE, %ah // interrupt function 
    mov $0, %bh   // page number
    mov $9, %bl   // attribute (white)
    int $0x10
    jmp PrintString
PrintString_return:
    retw
.endfunc

.func Reboot
Reboot:
    mPrintString reboot_msg
    xor %ax, %ax // interrupt function
    int $0x16    // wait for key 
    
    // machine language to jump to FFFF:0000 (reboot)
    .byte 0xEA   
    .word 0x0000
    .word 0xFFFF
.endfunc 

// %ax : LBA of the sector to read 
// %es:%bx : address of the buffer the sector is read to 
.func ReadSector
ReadSector:
    // %cx : current try count (we try 4 times in total)
    xor %cx, %cx

ReadSector_try:
    push %cx

    // calculate cylinder, track and sector 
    // LBA = (sector-1) + SPT*head + SPT*NH*cylinder
    // this gives the formulas :
    // cyclinder = (LBA / SectorsPerTrack) / NumHeads
    // sector    = (LBA mod SectorsPerTrack) + 1
    // head      = (LBA / SectorsPerTrack) mod NumHeads

    // BIOS interrupt
    // al : number of sectors to read
    // ah : interrupt function 
    // cx : sector number 
    // dh : head number 
    // dl : drive number 
    // es:bx : data buffer
    mov $0x0201, %ax     
    mov boot_drive, %dl // drive to read from

    int $0x13
    jc ReadSector_fail

    // Success !
    pop %cx
    retw 

ReadSector_fail:
    pop %cx 
    inc %cx 
    cmp $4, %cx 
    jge boot_fail

    // reset disk and try again 
    mResetDisk
    jmp ReadSector_try
.endfunc