.macro mGetIF
    pushf
    pop %eax 
    test $(1 << 9), %eax 
    jz 1f 
    mov $1, %eax 
    jmp 2f
1:  
    mov $0, %eax 
2:
.endm

.global get_interrupt_flag
get_interrupt_flag:
    mGetIF 
    ret

.global set_interrupt_flag
set_interrupt_flag:
    // save the old value of the interrupt flag
    mGetIF

    // update the interrupt flag    
    mov 4(%esp), %ebx
    test %ebx, %ebx 
    jz 1f
// enable
    sti 
    ret 
// disable 
1:  cli
    ret
    