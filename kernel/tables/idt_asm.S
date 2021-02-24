
.extern interrupt_handler

// arguments (stack) :
//   1) address of an 8 byte memory location containing 
//      the new contents of idtr
.global load_idtr
load_idtr:
    mov 4(%esp), %edx
    lidt (%edx)
    ret

.global enable_interrupts
enable_interrupts:
    sti 
    ret

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

// IRQs (for PICs)
// PIC 1
isr_no_errcode 32
isr_no_errcode 33
isr_no_errcode 34
isr_no_errcode 35
isr_no_errcode 36
isr_no_errcode 37
isr_no_errcode 38
isr_no_errcode 39

// PIC 2
isr_no_errcode 40
isr_no_errcode 41
isr_no_errcode 42
isr_no_errcode 43
isr_no_errcode 44
isr_no_errcode 45
isr_no_errcode 46
isr_no_errcode 47

// syscall
isr_no_errcode 128
