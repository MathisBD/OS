.extern handle_interrupt
.extern handle_syscall


// arguments (stack) :
//   1) address of an 8 byte memory location containing 
//      the new contents of idtr
.global load_idtr
load_idtr:
    mov 4(%esp), %edx
    lidt (%edx)
    ret


.macro mSaveState
    // save registers 
    // this has to respect the layout given by intr_frame_t
    // in interrupts/interrupts.h
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
.endm 

.macro mLoadKernelSel
    // load kernel data selector
    mov $0x10, %ax 
    mov %ax, %ds 
    mov %ax, %es 
    mov %ax, %fs
    mov %ax, %gs
.endm

.macro mCallHandler handler
    // get the address of the intr_frame_t struct 
    // (which we just contructed on the stack)
    mov %esp, %ebp

    // call the C syscall or interrupt handler,
    // aligning the stack.
    and $-16, %esp
    sub $12, %esp 
    push %ebp
    call \handler
    
    // restore the stack pointer.
    // we can't just pop into esp here,
    // since the C handler might modify its stack arguments.
    mov %ebp, %esp 
.endm

.macro mRestoreState 
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
.endm 

// the cpu pushed an error code
.macro mIsrErrcode n
.global isr\n
isr\n:
    cli
    push $\n // interrupt number
    mSaveState
    mLoadKernelSel
    mCallHandler handle_interrupt
    mRestoreState
    add $8, %esp // pop the interrupt number and error code
    iret // pops the stuff the cpu pushed 
.endm

// the cpu didn't push an error code
.macro mIsrNoErrcode n
.global isr\n
isr\n:
    cli
    push $0 // dummy error code 
    push $\n // interrupt number
    mSaveState
    mLoadKernelSel
    mCallHandler handle_interrupt
    mRestoreState
    add $8, %esp // pop the interrupt number and error code
    iret // pops the stuff the cpu pushed 
.endm

// the cpu didn't push an error code
.macro mIsrSyscall n
.global isr\n
isr\n:
    cli
    push $0 // dummy error code 
    push $\n // interrupt number
    mSaveState
    mLoadKernelSel

    // interrupts are enabled during a syscall
    sti 
    mCallHandler handle_syscall

    cli
    mRestoreState
    add $8, %esp // pop the interrupt number and error code
    iret // pops the stuff the cpu pushed 
.endm

mIsrNoErrcode 0   // Divide Error
mIsrNoErrcode 1   // Debug Exception
mIsrNoErrcode 2   // Non Maskable Interrupt 
mIsrNoErrcode 3   // Breakpoint
mIsrNoErrcode 4   // Overflow
mIsrNoErrcode 5   // Out of Bounds
mIsrNoErrcode 6   // Invalid Opcode
mIsrNoErrcode 7   // Device Not Available
mIsrErrcode 8     // Double Fault
mIsrNoErrcode 9   // Coprocessor Segment Overrun
mIsrErrcode 10    // Invalid TSS
mIsrErrcode 11    // Segment Not Present
mIsrErrcode 12    // Stack Fault
mIsrErrcode 13    // General Protection
mIsrErrcode 14    // Page Fault 
mIsrNoErrcode 15  // Intel reserved
mIsrNoErrcode 16  // Floating Point Error
mIsrErrcode 17    // Alignment Check
mIsrNoErrcode 18  // Machine Check

// Intel reserved
mIsrNoErrcode 19
mIsrNoErrcode 20
mIsrErrcode 21
mIsrNoErrcode 22
mIsrNoErrcode 23
mIsrNoErrcode 24
mIsrNoErrcode 25
mIsrNoErrcode 26
mIsrNoErrcode 27
mIsrNoErrcode 28
mIsrNoErrcode 29
mIsrNoErrcode 30
mIsrNoErrcode 31

// IRQs (for PICs)
// PIC 1
mIsrNoErrcode 32
mIsrNoErrcode 33
mIsrNoErrcode 34
mIsrNoErrcode 35
mIsrNoErrcode 36
mIsrNoErrcode 37
mIsrNoErrcode 38
mIsrNoErrcode 39

// PIC 2
mIsrNoErrcode 40
mIsrNoErrcode 41
mIsrNoErrcode 42
mIsrNoErrcode 43
mIsrNoErrcode 44
mIsrNoErrcode 45
mIsrNoErrcode 46
mIsrNoErrcode 47

// syscall 
mIsrSyscall 128