
.macro mInitSegments
    cli
    mov %dl, boot_drive // %dl contains the drive number
    mov $0, %ax 
    mov %ax, %ds 
    mov %ax, %es
    mov %ax, %ss 
    mov $0x7c00, %sp 
    sti 
.endm

.macro mResetDisk
    mov boot_drive, %dl 
    xor %ax, %ax 
    int $0x13
    jc boot_fail // carry set == error
.endm

.macro mGetDiskGeometry
    mov $0x8, %ah // interrupt subfunction
    mov boot_drive, %dl
    int $0x13
    jc boot_fail
    // returns dh = num_heads - 1
    // cl & 0x3f = sectors_per_track  
    inc %dh 
    mov %dh, num_heads 
    and $0x3f, %cl 
    mov %cl, sectors_per_track
.endm 

// prints the hex digit in the lower 4 bits of al
.macro mPrintHexDigit
    push %ax 
    push %bx 

    add $0x30, %al 
    cmp $0x39, %al 
    jle 1f

    add $0x7, %al
1:
    mov $0xE, %ah // interrupt function 
    mov $0, %bh   // page number
    mov $9, %bl   // attribute (white)
    int $0x10

    pop %bx 
    pop %ax 
.endm 

.macro mPrintString str 
    lea \str, %si 
    call PrintString 
.endm

.macro mPrintMem mem count 
    lea \mem, %si 
    mov $\count, %cx 
    call PrintMem 
.endm 

.macro mReboot
    call Reboot
.endm

.macro mReadSector lba load_segment
    mov $\lba, %ax 
    mov $\load_segment, %bx
    mov %bx, %es 
    xor %bx, %bx 
    call ReadSector
.endm

.macro mGetFilesystemInfo
    // load the superblock 
    mov $TMP_LOAD_SEGMENT, %bx 
    mov %bx, %es 
    xor %bx, %bx 
    mov $2, %ax // 2 = SUPERBLOCK_OFFSET / SECTOR_SIZE
    call ReadSector
    
    // si : superblock base
    mov $(16 * TMP_LOAD_SEGMENT), %si
    
    mov 40(%si), %ax 
    mov %ax, inodes_per_group
    
    mov 24(%si), %cx // log_block_size (0 means 1024, 1 means 2048, etc)
    // a block spans 1 << (log_block_size + 1) sectors
    inc %cl 
    mov $1, %ax 
    sal %cl, %ax
    mov %ax, sectors_per_block
.endm

// loads an inode specified by the number in ax
// in segment TMP_LOAD_SEGMENT (though not necessarily at the 
// start of the segment).
// returns the offset of the inode from 16*TMP_LOAD_SEGMENT
// in ax
.macro mLoadInode
    // block group : (inode - 1) / inodes_per_group
    // div : dx:ax/bx --> ax, dx
    dec %ax
    xor %dx, %dx 
    mov inodes_per_group, %bx 
    div %bx 
    // ax = block group
    // dx = offset in block group 
    push %dx
    
    sal $5, %ax // 2**5 = 32 bytes : size of a table entry 
    add $SUPERBLOCK_OFFSET, %ax 
    xor %dx, %dx 
    mov $512, %bx // 512 : sector size
    div %bx 

    // skip the superblock block 
    // which spans 1 << (log_block_size + 1) sectors 
    mov log_block_size, %cl
    inc %cl 
    mov $1, %bx 
    sal %cl, %bx
    add %bx, %ax 
    // ax = sector containing table entry 
    // dx = offset of entry in sector 

    // load the sector
    mov $TMP_LOAD_SEGMENT, %bx 
    mov %bx, %es 
    xor %bx, %bx 
    push %dx 
    call ReadSector 
    pop %dx 

    // si : now points to the table entry
    mov $(16 * TMP_LOAD_SEGMENT), %si
    add %dx, %si     
    mov 8(%si), %cx
    imul sectors_per_block, %cx  
    // cx : sector of the inode table 

    pop %ax // get the inode index in the block group
    sal $7, %ax // 2**7 = 128 bytes : size of an inode
    // ax = offset of the inode from the start of the inode table
    xor %dx, %dx 
    mov $512, %bx 
    div %bx 
    add %cx, %ax     
    // ax = sector containing inode 
    // dx = offset of inode in sector 

    xor %bx, %bx 
    push %dx 
    call ReadSector 
    pop %ax 
.endm

