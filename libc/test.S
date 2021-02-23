.global _main

.section .data
x: .int 42

.section .text
_main:
    mov $0xDEADBABA, %eax
L1:
    jmp L1


