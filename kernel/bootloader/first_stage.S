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

// BOOT INFO to load the second stage
// this is filled in when the disk image is created
// it has to be at offset 0x02 bytes in the bootsector
s2_inode: .int 15
  

// INSTRUCTIONS
load:   
    mInitSegments
    mCheckExtensions
    mResetDisk  
    mGetDiskGeometry
    mGetFilesystemInfo
    
    mov s2_inode, %ax 
    mLoadInode  

    mov $(16 * TMP_LOAD_SEGMENT), %si 
    add %ax, %si
    mov 40(%si), %si     
    imul sectors_per_block, %si 
    // si = sector to load
    
    mov $S2_LOAD_SEGMENT, %di 
    mov %di, %es 
    xor %di, %di     
    call ReadSector

    ljmp $S2_LOAD_SEGMENT, $0

    //mLoadInodeContentsShort

halt:  
    cli
    hlt

boot_fail: 
    mReboot


// FUNCTIONS
.include "bootloader/functions.S"
 

// DATA (after the instructions of course)
reboot_msg: .asciz "ERROR-Press a key to reboot\n\r"
// disk info  
boot_drive: .byte 0   
sectors_per_track: .byte 0
num_heads: .byte 0
// disk address packet (input to bios interrupt 0x13
// to read disk using 48bit LBA)
dap_size: .byte 16
dap_zero: .byte 0
dap_sector_count: .short 0 // max 127
dap_buffer_offset: .short 0
dap_buffer_segment: .short 0 
dap_lba_low: .int 0
dap_lba_high: .int 0
// filesystem info (ext2)
inodes_per_group: .int 0 
sectors_per_block: .short 0
block_size: .int 0

tmp: .short 0

s1_code_end:
// pad up to 510 bytes (leave space for the boot magic number)
.fill (510 - (s1_code_end - s1_code_start)), 1, 0
boot_magic: .int 0xAA55 // for the bios to recognize us

 