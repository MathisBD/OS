.global keyboard_intr
.extern keyboard_intr_handler
.global keyboard_intr_handler

keyboard_intr:
    pusha
    call keyboard_intr_handler
    popa
    iret
    