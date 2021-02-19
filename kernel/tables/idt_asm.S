.global load_idtr

.global default_pic1_isr
.global default_pic2_isr

.extern pic_eoi
.extern interrupt_handler

// arguments (stack) :
//   1) address of an 8 byte memory location containing 
//      the new contents of idtr
load_idtr:
    mov 4(%esp), %edx
    lidt (%edx)
    sti // enable interrupts
    ret

// irq 0 to 7
default_pic1_isr:
    pusha
    push $0
    call pic_eoi
    add $4, %esp
    popa
    iret

// irq 8 to 15
default_pic2_isr:
    pusha
    push $8
    call pic_eoi
    add $4, %esp
    popa
    iret
 
.section .text

isr_common:
    // save registers 
    push %edi
    push %esi
    push %eax 
    push %ebx 
    push %ecx 
    push %edx 
    push %ebp
    push %ds 
    push %es 
    push %fs 
    push %gs

    // load kernel data selector
    mov $0x10, %ax 
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs
    mov %ax, %gs

    // call the C interrupt handler 
    // push its argument : address of the user_regs struct
    // (which happens to be on the stack)
    mov %esp, %ecx
    push %ecx
    call interrupt_handler
    add $4, %esp // remove the address of user_regs struct

    // restore registers 
    pop %gs 
    pop %fs 
    pop %es 
    pop %ds 
    pop %ebp 
    pop %edx
    pop %ecx 
    pop %ebx 
    pop %eax 
    pop %esi 
    pop %edi 

    add $8, %esp // pop the interrupt number and error code
    iret // pops the stuff the cpu pushed 

.macro isr_no_errcode n
.global isr\n
isr\n:
    push $0 // dummy error code 
    push $\n // interrupt number
    jmp isr_common
.endm

.macro isr_errcode n 
.global isr\n
isr\n:
    push $\n
    jmp isr_common
.endm

isr_no_errcode 0  // Divide Error
isr_no_errcode 1  // Debug Exception
isr_no_errcode 2  // Non Maskable Interrupt 
isr_no_errcode 3  // Breakpoint
isr_no_errcode 4  // Overflow
isr_no_errcode 5  // Out of Bounds
isr_no_errcode 6  // Invalid Opcode
isr_no_errcode 7  // Device Not Available
isr_errcode 8     // Double Fault
isr_no_errcode 9  // Coprocessor Segment Overrun
isr_errcode 10    // Invalid TSS
isr_errcode 11    // Segment Not Present
isr_errcode 12    // Stack Fault
isr_errcode 13    // General Protection
isr_errcode 14    // Page Fault 
isr_no_errcode 15 // Intel reserved
isr_no_errcode 16 // Floating Point Error
isr_errcode 17    // Alignment Check
isr_no_errcode 18 // Machine Check

// Intel reserved
isr_no_errcode 19
isr_no_errcode 20
isr_errcode 21
isr_no_errcode 22
isr_no_errcode 23
isr_no_errcode 24
isr_no_errcode 25
isr_no_errcode 26
isr_no_errcode 27
isr_no_errcode 28
isr_no_errcode 29
isr_no_errcode 30
isr_no_errcode 31
