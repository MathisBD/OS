.global _main

.section .text
_main:
    mov %esp, %ebx

    .extern printf
    push %ebx
    push $str 
    xor %eax, %eax 
    call printf
    add $8, %esp 

    mov 4(%esp), %ebx 
    push %ebx
    push $format
    xor %eax, %eax 
    call printf 
    add $8, %esp


    mov $0xDEADBABA, %eax 
    int $0x80
L1: jmp L1


str: .asciz "test : %x\n"
format: .asciz "test argc=%d\n"



