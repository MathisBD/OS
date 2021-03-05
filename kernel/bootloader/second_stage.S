.code16  // we are in 16-bit real mode
// adress we are loaded at by the first stage bootloader
.org 0x8000
.text 

// size of the second stage file 
// this way it always fits in the direct blocks of its inode
.set SECOND_STAGE_SIZE, 12*1024 
.set STACK_TOP, 0x7000

.include "bootloader/macros.S"

// the first stage passes us some data in memory :
// ax = start address of the data 
// bx = end address of the data
s2_code_start:
    mResetSegments
    mCopyFirstStageData

    //mResetDisk 
    mEnableA20
    //mSetupIDT 
    //mSetupGDT 


halt:
    cli 
    hlt

// DATA 
msg: .asciz "Second stage yay !!!\n\r"
s2_shared_data:
mSharedData



.include "bootloader/functions.S"
.include "bootloader/functions_extra.S"
.include "bootloader/a20.S"

s2_code_end:

.fill (SECOND_STAGE_SIZE - (s2_code_end - s2_code_start)), 1, 0
