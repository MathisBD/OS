.global main

.section .text
main:
    .extern printf
    pushf
    push $str 
    xor %eax, %eax 
    call printf

    mov $0xDEADBABA, %eax 
    int $0x80
L1: jmp L1


str: .asciz "test : %x\n"



