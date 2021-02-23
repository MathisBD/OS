.global loader_jump

// arguments : 
//   1) address to jump to
loader_jump:
    mov 4(%esp), %ecx
    jmp *%ecx