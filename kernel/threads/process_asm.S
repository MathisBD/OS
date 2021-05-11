#include "tables/selectors.h"

.global exec_jump_asm

// arguments : 
//   1) program entry address
//   2) address of user stack top
//   3) argc 
//   4) argv
exec_jump_asm:
    cli // interrupts will be reenabled with iret

    push %ebp
    mov %esp, %ebp 

    // prepare the initial stack frame for the new program
    push 20(%ebp)   // argv
    push 16(%ebp)   // argc
    sub $4, %esp    // dummy return address

    // prepare the stack for iret.
    // the user RPL in CS will tell the cpu 
    // to change ring on iret, and thus pop esp and ss 
    // from the stack.
    push  $(USER_DATA_SEL | RPL_USER)  // user SS
    push 12(%ebp)       // user ESP 
    
    pushf               // EFLAGS
    orw $0x200, (%esp)  // enable interrupt flag
    push $(USER_CODE_SEL | RPL_USER) // CS 
    push 8(%ebp)        // EIP 

    // data selectors
    mov $(USER_DATA_SEL | RPL_USER), %ax 
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs

    iret
    