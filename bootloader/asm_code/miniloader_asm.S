

// arguments (stack) :
//   1) 32-bit address to jump to
.global jump_to_kernel
jump_to_kernel:
    mov 4(%esp), %ecx 
    jmp *%ecx 

