
// expects a string in ds:si
.func PrintString
PrintString:
    cld
    lodsb
    or %al, %al 
    jz PrintString_return

    mov $0xE, %ah // interrupt function 
    mov $0, %bh   // page number
    mov $9, %bl   // attribute (white)
    int $0x10
    jmp PrintString
PrintString_return:
    ret
.endfunc

// expects an adress in ds:si 
// and a count in cx
.func PrintMem
PrintMem:
    or %cx, %cx 
    jz PrintMem_return 

    cld
    lodsb 
    dec %cx 

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
    mov $0x20, %al
    mov $0xE, %ah // interrupt function 
    mov $0, %bh   // page number
    mov $9, %bl   // attribute (white)
    int $0x10

    jmp PrintMem
PrintMem_return:
    ret 
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
    push %ax // LBA  
    push %cx // try number 
    push %bx // data buffer address 
    
    // calculate cylinder, track and sector 
    // LBA = (sector-1) + SPT*head + SPT*NH*cylinder
    // this gives the formulas :
    // cylinder = (LBA / SectorsPerTrack) / NumHeads
    // sector    = (LBA mod SectorsPerTrack) + 1
    // head      = (LBA / SectorsPerTrack) mod NumHeads

    // divide (dx:ax/bx to ax, dx)
    xor %dx, %dx
    mov sectors_per_track, %bx 
    div %bx 
    // ax = LBA / SectorsPerTrack
    // dx = LBA mod SectorsPerTrack 
    inc %dx 
    mov %dl, %cl // cl = sector (for int 0x13)

    mov head_num, %bx 
    xor %dx, %dx 
    div %bx 
    // ax = cylinder 
    // dx = head
    mov %al, %ch // ch = cylinder (for int 0x13)
    mov %dl, %dh // dh = head (for int 0x13)

    // BIOS interrupt
    // al : number of sectors to read
    // ah : interrupt function 
    // cl : sector number 
    // ch : cylinder number
    // dh : head number 
    // dl : drive number 
    // es:bx : data buffer
    mov $0x0201, %ax     
    mov boot_drive, %dl // drive to read from
    pop %bx // restore data buffer address

    int $0x13
    jc ReadSector_fail

    // Success !
    pop %cx
    pop %ax 
    retw

ReadSector_fail:
    pop %cx 
    inc %cx 
    cmp $4, %cx 
    jge boot_fail

    // reset disk and try again 
    mResetDisk
    pop %ax // do this after mResetDisk (it clears ax)
    jmp ReadSector_try
.endfunc
