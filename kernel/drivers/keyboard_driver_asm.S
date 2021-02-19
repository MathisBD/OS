.global keyboard_isr
.extern keyboard_isr_c

keyboard_isr:
    pusha
    call keyboard_isr_c
    popa
    iret
    