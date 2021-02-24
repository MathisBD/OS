.global _main

.section .text
_main:
    mov $0xDEADBABA, %eax
    int $128
L1:
    jmp L1


