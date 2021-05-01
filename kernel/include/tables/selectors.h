#pragma once

// only #defines in this file,
// it will be #included in assembly files.

// segment selectors
#define KERNEL_CODE_SEL     0x08
#define KERNEL_DATA_SEL     0x10
#define USER_CODE_SEL       0x18
#define USER_DATA_SEL       0x20
#define TSS_SEL             0x28

// requested privilege level.
// OR this with the segment selector
#define RPL_KERNEL  0x00
#define RPL_USER    0x03
