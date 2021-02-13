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

    // initialize interrupts
    // get the adress of the IDT in %eax
    sub $8, %esp
    sidt (%esp)
    mov 2(%esp), %eax
    add $8, %esp
    // store an interrupt handler (keyboard ?)
    mov $0, %eax
    movl $int_handler, 0x24(%eax)

    // start the kernel
    call kernel_main
// in case the kernel returns (it shouldn't)
hang:
    cli // disable cpu interrupts
    hlt // halt the cpu
    jmp hang // if still running, try again


    .global int_handler
    .align 4
int_handler:
    pushal
    cld 
    call c_int_handler
    popal
    iret
