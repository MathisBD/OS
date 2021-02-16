.global load_idtr

.global default_pic1_isr
.global default_pic2_isr

.global isr0
.global isr1
.global isr2
.global isr3
.global isr4
.global isr5
.global isr6
.global isr7

.global isr8
.global isr9
.global isr10
.global isr11
.global isr12
.global isr13
.global isr14
.global isr15

.global isr16
.global isr17
.global isr18
.global isr19
.global isr20
.global isr21
.global isr22
.global isr23

.global isr24
.global isr25
.global isr26
.global isr27
.global isr28
.global isr29
.global isr30
.global isr31

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
 

.section .data
// Registers struct used to store the user data (see idt.c).
// We can't pass it on the stack as the compiler may
// produce code that corrupts some stack values
// that are arguments of the C interrupt handler but that 
// it does not use.
// Note that the C interrupt handler may modify some values in 
// this struct to pass info back to the user.
// No need to store esp, eip, eflags, cs or ss :
// they are pushed by the cpu when an interrupt is triggered
// and popped back when the interrupt returns.
.align 4
user_regs:
    ds: .long
    edi: .long
    esi: .long 
    ebp: .long 
    ebx: .long 
    edx: .long 
    ecx: .long 
    eax: .long 
    intr_num: .long 
    error_code: .long 

.section .text

isr_common:
    // save user registers and info for the interrupt handler
    mov %edi, (edi)
    mov %esi, (esi)
    mov %ebp, (ebp)
    mov %ebx, (ebx)
    mov %edx, (edx)
    mov %ecx, (ecx)
    mov %eax, (eax)

    mov %ds, %ax 
    mov %eax, (ds)

    pop %ebx 
    mov %ebx, (intr_num)
    pop %ebx
    mov %ebx, (error_code)

    // load the kernel data selector
    mov $0x10, %ax
    mov %ax, %ds
    mov %ax, %es 
    mov %ax, %fs
    mov %ax, %gs

    push $user_regs
    call interrupt_handler
    add $4, %esp

    // reload the original (user) data descriptors
    mov (ds), %eax
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs 
    mov %ax, %gs

    // restore the user registers
    // (potentially modified by the C interrupt handler)
    mov (edi), %edi 
    mov (esi), %esi 
    mov (ebp), %ebp 
    mov (ebx), %ebx
    mov (edx), %edx 
    mov (ecx), %ecx
    mov (eax), %eax

    iret 



isr0:
    push $0 // dummy error code
    push $0 // interrupt number
    jmp isr_common
isr1:
    push $0
    push $1 
    jmp isr_common
isr2:
    push $0
    push $2 
    jmp isr_common
isr3:
    push $0
    push $3 
    jmp isr_common
isr4:
    push $0
    push $4 
    jmp isr_common
isr5:
    push $0
    push $5 
    jmp isr_common
isr6:
    push $0
    push $6 
    jmp isr_common
isr7:
    push $0
    push $7 
    jmp isr_common

isr8:
    push $0
    push $8 
    jmp isr_common
isr9:
    push $0
    push $9 
    jmp isr_common
isr10:
    push $0
    push $10 
    jmp isr_common
isr11:
    push $0
    push $11 
    jmp isr_common
isr12:
    push $0
    push $12 
    jmp isr_common
isr13:
    push $0
    push $13 
    jmp isr_common
isr14:
    push $0
    push $14 
    jmp isr_common
isr15:
    push $0
    push $15 
    jmp isr_common

isr16:
    push $0
    push $16 
    jmp isr_common
isr17:
    // THE CPU PUSHES AN ERROR CODE FOR ISR 17
    push $17 
    jmp isr_common
isr18:
    push $0
    push $18 
    jmp isr_common
isr19:
    push $0
    push $19 
    jmp isr_common
isr20:
    push $0
    push $20 
    jmp isr_common
isr21:
    // THE CPU PUSHES AN ERROR CODE FOR ISR 21
    push $21 
    jmp isr_common
isr22:
    push $0
    push $22 
    jmp isr_common
isr23:
    push $0
    push $23 
    jmp isr_common

isr24:
    push $0
    push $24 
    jmp isr_common
isr25:
    push $0
    push $25 
    jmp isr_common
isr26:
    push $0
    push $26 
    jmp isr_common
isr27:
    push $0
    push $27 
    jmp isr_common
isr28:
    push $0
    push $28 
    jmp isr_common
isr29:
    push $0
    push $29 
    jmp isr_common
isr30:
    push $0
    push $30 
    jmp isr_common
isr31:
    push $0
    push $31 
    jmp isr_common

    
