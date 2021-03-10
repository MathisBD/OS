

// arguments (stack) :
//   1) 32-bit address to jump to
//   2) address of the boot_info structure
.global jump_to_kernel
jump_to_kernel:
    mov 8(%esp), %eax // BOOT INFO ADDRESS : passed in EAX 
    mov 4(%esp), %ecx // entry point  
    jmp *%ecx
    ret 

