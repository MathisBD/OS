.global main

.section .text
main:
    push $8
    mov $0xDEADBABA, %eax
    int $0x80
L1: jmp L1


