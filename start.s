.extern kernel_main

.global start

// GRUB constants
.set MB_MAGIC, 0x1BADB002
// load modules on page boundaries + provide memory map
.set MB_FLAGS, (1 << 0) | (1 << 1) 
.set MB_CHECKSUM, (0 - (MB_MAGIC + MB_FLAGS))


.section .multiboot
    .align 4 
    .long MB_MAGIC
    .long MB_FLAGS
    .long MB_CHECKSUM


.section .bss
    // stack space (4096 bytes for now)
    .align 16
stack_bottom:
    .skip 4096
stack_top:


.section .text
start:
    // initialize stack
    mov $stack_top, %esp
    call kernel_main
// in case the kernel returns (it shouldn't)
hang:
    cli // disable cpu interrupts
    hlt // halt the cpu
    jmp hang // if still running, try again
