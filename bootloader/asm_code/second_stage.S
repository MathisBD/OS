.code16  // we are in 16-bit real mode
// adress we are loaded at by the first stage bootloader
.org 0x8000
.text 

// size of the second stage file 
// this way it always fits in the direct blocks of its inode
.set SECOND_STAGE_SIZE, 12*1024 
.set IDT_START, 0x0500
.set GDT_START, 0x0D00
.set STACK_TOP, 0x4000
// needs to be named the same as in the first stage
.set TMP_LOAD_SEGMENT, 0x400
// the second stage is at 0000:8000


// the first stage passes us some data in memory :
// ax = start address of the data 
// bx = end address of the data
s2_code_start:
    jmp entry
    .include "asm_code/macros.S"
    .include "asm_code/functions.S"
    .include "asm_code/functions_extra.S"
    .include "asm_code/a20.S"

entry:
    mResetSegments
    mCopyFirstStageData
    mSaveKernelInode
    mResetDisk 
    mEnableA20
    mSetupGDT 
    mEnterPMode
halt:
    cli 
    hlt
boot_fail:
    mReboot

// DATA 
msg: .asciz "Second stage yay !!!\n\r"
a20_error_msg: .asciz "Could not enable line A20\n\r"
reboot_msg: .asciz "ERROR - Press any key to reboot\n\r"
s2_shared_data:
mSharedData

idtr_contents: 
    .word (256*8 - 1) // size 
    .int IDT_START 
gdtr_contents: 
    .word (3*8 - 1)   // size 
    .int GDT_START
kernel_inode_contents:
    .skip 128

// 32 bit part of the second stage 
.code32
protected_mode_entry:
    // setup segment selectors 
    mov $0x10, %ax 
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs 
    mov %ax, %ss 
    // reset stack
    mov $STACK_TOP, %esp
    // align it on 8 mod 16
    sub $0x8, %esp
    
    // transition to C code
    .extern s2_main 
    call s2_main 

    cli 
    hlt
