.extern kernel_main
.extern get_mmap
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
    cli
    mov $stack_top, %esp // initialize stack
    
    // GRUB puts some values in %ebx and %eax
    push %eax
    push %ebx
    call get_mmap // get the mmap that GRUB setup
    add $8, %esp

    call kernel_main // start the kernel

    // this should not happen
    cli
    hlt

