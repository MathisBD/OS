.code16  // we are in 16-bit real mode

// this is the address the bootloader is loaded at.
// we have to trim the first 0x7c00 bytes to get the 
// boot sector and copy it to the first disk sector
.org 0x7C00
// The memory below 0x500 is reserved for the BIOS.
// The memory between 0x9FC00 and 0xA0000 is reserved for the BIOS.
// The memory above 0xA0000 is BIOS and mmap IO (up to 1MB)
// The memory above 1MB is used by the kernel 
// see this document (page 5) : https://www.cs.cmu.edu/~410-s07/p4/p4-boot.pdf

.text

// segment the second stage will be loaded in 
.set S2_LOAD_SEGMENT, 0x800
// segment used by the first stage bootloader to load temporary
// sectors from disk (superblock, inode table, etc)
.set TMP_LOAD_SEGMENT, 0x700
.set STACK_TOP, 0x7000
// offset (in bytes) in the hard drive of the superblock
.set SUPERBLOCK_OFFSET, 1024

.include "bootloader/macros.S"

s1_code_start: 
    jmp load

// BOOT INFO to load the second stage and kernel
// this is filled in when the disk image is created
// it has to be at offset 0x02 bytes in the bootsector
s2_inode: .int 15
kernel_inode_input: .int 14 

// INSTRUCTIONS
load:   
    // save boot drive
    mov %dl, boot_drive
    mResetSegments
    mCheckExtensions
    mResetDisk  
    mGetDiskGeometry
    mGetFilesystemInfo

    mov kernel_inode_input, %ax 
    mov %ax, kernel_inode 
    mov kernel_inode_input+2, %ax 
    mov %ax, kernel_inode+2

    mov s2_inode, %ax 
    mLoadInode  
    mov $(16 * TMP_LOAD_SEGMENT), %bx
    add %ax, %bx
    mLoadSecondStage
    mJumpToS2
halt:  
    cli
    hlt

boot_fail: 
    mReboot


// FUNCTIONS
.include "bootloader/functions.S"
 

// DATA (after the instructions of course)
reboot_msg: .asciz "ERROR - Press any key to reboot\n\r"

// data passed to the second stage
s1_shared_data_start:
mSharedData
s1_shared_data_end: 

s1_code_end:
// pad up to 510 bytes (leave space for the boot magic number)
.fill (510 - (s1_code_end - s1_code_start)), 1, 0
boot_magic: .int 0xAA55 // for the bios to recognize us

 