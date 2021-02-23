.global _main

.section .text
_main:
    mov $0xDEADBABA, %eax
L1:
    jmp L1


