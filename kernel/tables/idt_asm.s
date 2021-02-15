.global load_idtr
.global default_pic1_intr
.global default_pic2_intr
.extern pic_eoi
 
// arguments (stack) :
//   1) address of an 8 byte memory location containing 
//      the new contents of idtr
load_idtr:
    mov 4(%esp), %edx
    lidt (%edx)
    sti // enable interrupts
    ret

// irq 0 to 7
default_pic1_intr:
    push $0
    call pic_eoi
    add $4, %esp
    iret

// irq 8 to 15
default_pic2_intr:
    push $8
    call pic_eoi
    add $4, %esp
    iret
 
