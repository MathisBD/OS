.global loader_jump_user

// arguments : 
//   1) address to jump to
//   2) address of user stack top
loader_jump_user:
    mov 4(%esp), %ecx
    mov 8(%esp), %edx

    cli // interrupts will be reenabled with iret
    
    //mov $(0x10), %ax
    mov $(0x20 | 0x03), %ax 
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs

    push $(0x20 | 0x03)     // user SS
    push $0                 // user ESP
    push $(1 << 9 | 1 << 1) // user EFLAGS
    push $(0x18 | 0x03)     // user CS
    //push $(0x08)
    push %ecx               // user EIP 
    iret