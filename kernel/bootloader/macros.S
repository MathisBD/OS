
// this block must be EXACTLY the same 
// in the first and second stages
.macro mSharedData
// disk info  
boot_drive: .byte 0   
sectors_per_track: .byte 0
num_heads: .byte 0
// filesystem info (ext2)
inodes_per_group: .int 0 
sectors_per_block: .short 0
block_size: .int 0
// kernel inode 
kernel_inode: .int 0
.endm

.macro mResetSegments
    cli
    mov $0, %bp 
    mov %bp, %ds 
    mov %bp, %es
    mov %bp, %ss 
    mov $STACK_TOP, %sp 
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

.macro mCheckExtensions
    mov $0x41, %ah 
    mov $0x55AA, %bx 
    mov boot_drive, %dl 
    int $0x13
    jc boot_fail 
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

.macro mReboot
    call Reboot
.endm

.macro mGetFilesystemInfo
    // load the superblock 
    mov $TMP_LOAD_SEGMENT, %di
    mov %di, %es 
    xor %di, %di 
    mov $2, %si // 2 = SUPERBLOCK_OFFSET / SECTOR_SIZE
    call ReadSector
    
    // si : superblock base
    mov $(16 * TMP_LOAD_SEGMENT), %si
    
    mov 40(%si), %ax 
    mov %ax, inodes_per_group
    
    mov 24(%si), %cx // log_block_size (0 means 1024, 1 means 2048, etc)
    mov %cl, log_block_size
    // a block spans 1 << (log_block_size + 1) sectors
    inc %cl 
    mov $1, %ax 
    sal %cl, %ax
    mov %ax, sectors_per_block

    imul $512, %ax 
    mov %ax, block_size
.endm

// loads an inode specified by the number in ax
// in segment TMP_LOAD_SEGMENT (though not necessarily at the 
// start of the segment).
// returns the offset of the inode in ax
.macro mLoadInode
    // calculte block group = (inode - 1) / inodes_per_group
    // div : dx:ax/bx --> ax, dx
    dec %ax
    xor %dx, %dx 
    mov inodes_per_group, %bx 
    div %bx 
    // ax = block group
    // dx = index in block group 
    push %dx
    
    // calculate sector containing block group table entry
    sal $5, %ax // 2**5 = 32 bytes : size of a table entry
    xor %dx, %dx 
    mov $512, %bx // 512 : sector size
    div %bx 
    // skip the boot block and superblock
    add sectors_per_block, %ax 
    cmpw $2, sectors_per_block
    jne 1f
    // special case : the block size is 1024, skip a second block 
    add $2, %ax
1:  
    // ax = sector containing table entry 
    // dx = offset of entry in sector 

    // load the sector
    mov $TMP_LOAD_SEGMENT, %di 
    mov %di, %es 
    xor %di, %di 
    mov %ax, %si
    push %dx 
    call ReadSector 
    pop %dx 

    // si : now points to the group table entry
    mov $(16 * TMP_LOAD_SEGMENT), %si
    add %dx, %si     
    mov 8(%si), %cx
    imul sectors_per_block, %cx  
    // cx : sector of the inode table 

    pop %ax // get the inode index in the block group
    imul $128, %ax // 128 bytes : size of an inode
    // ax = offset of the inode from the start of the inode table
    xor %dx, %dx 
    mov $512, %bx 
    div %bx 
    add %cx, %ax     
    // ax = sector containing inode 
    // dx = offset of inode in sector 

    xor %di, %di 
    mov %ax, %si
    push %dx 
    call ReadSector 
    pop %ax
.endm


// expects a pointer to the inode in bx
// assumes the inode data occupies exactly 12KB
.macro mLoadSecondStage
    add $40, %bx
    // bx = pointer to the block number to load

    mov $S2_LOAD_SEGMENT, %di 
    mov %di, %es 
    xor %di, %di

    mov (%bx), %si     
    imul sectors_per_block, %si 
    // si = sector to load

    mov sectors_per_block, %dx 
    mov $24, %cx // the file is 12KB, so 24 sectors long
    // cx = number of sectors to load
    // dx = number of sectors to load in current block
1:
    test %cx, %cx 
    jz 4f

    mov (%bx), %ax 
    test %ax, %ax 
    jnz 2f

    // zero out sector 
    push %cx 
    push %di 

    // es:di is already set 
    mov $0, %al     // here we can overwrite ax  
    mov $512, %cx   // sector size
    cld
    rep stosb

    pop %di // stosb changes di
    pop %cx 
    jmp 3f

2:  // load sector from memory 
    push %dx 
    push %cx      
    push %bx 
    call ReadSector
    pop %bx 
    pop %cx 
    pop %dx 

3:  // advance to the next sector/block
    dec %cx
    dec %dx  
    inc %si 
    add $512, %di 

    test %dx, %dx 
    jnz 1b 

    // advance to next block  
    add $4, %bx
    mov (%bx), %si 
    imul sectors_per_block, %si
    mov sectors_per_block, %dx 
    jmp 1b 
4:  // finished
.endm 

.macro mJumpToS2 
    lea s1_shared_data_start, %ax 
    lea s1_shared_data_end, %bx
    ljmp $S2_LOAD_SEGMENT, $0
.endm

// ax = start of first stage shared data 
// bx = end of first stage shared data 
.macro mCopyFirstStageData
    mov %ax, %si 
    lea s2_shared_data, %di 
    mov %bx, %cx 
    sub %ax, %cx 
    cld
    rep movsb
.endm 

.macro mEnableA20
    call CheckA20
    
.endm