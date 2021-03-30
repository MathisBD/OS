
.global switch_asm 
// when forking a process, set ctx.eip to this 
// address for the new process
.global switch_asm_fork 


// field offsets in the proc_ctx_t struct
ESP_OFS=0
ESP0_OFS=4
EIP_OFS=8


// arguments : 
// 1) prev : proc_desc*
// 2) next : proc_desc*
// 3) last : proc_desc*
// 4) ctx_ofs : offset of the ctx field in a proc_desc
.global switch_asm
switch_asm:
    // at the start, we are executing process A
    mov 16(%esp), %ecx 
    mov 4(%esp), %eax // prev from A's stack
    mov 8(%esp), %edx // next from A's stack
 
    // push on A's stack    
    pushfl
    push %ebp
    push %ebx

    // when returning to prev, jump there
    mov $switch_asm_regular, EIP_OFS(%eax, %ecx, 1)

    // change stacks from A to B
    mov %esp, ESP_OFS(%eax, %ecx, 1)
    mov ESP_OFS(%edx, %ecx, 1), %esp

    // we are now executing process B 
    .extern curr_proc 
    mov %edx, curr_proc

    // update the TSS esp 
    mov ESP0_OFS(%edx, %ecx, 1), %ebx 
    push %ebx 
    .extern set_tss_esp // defined in gdt.h
    call set_tss_esp
    pop %ebx

    // most of the time, next->ctx.eip is switch_asm_regular
    // except : when next hasn't run at all yet
    mov EIP_OFS(%edx, %ecx, 1), %ebx 
    jmp *%ebp 

// here is where prev returns
// AND where next starts its execution if it was
// previoulsy switched out
switch_asm_regular:
    pop %ebx
    pop %ebp 
    popfl

    // update last
    mov %eax, 12(%esp)

    // use the return adress on the stack, setup by gcc
    ret 

// here is where next starts executing if it was
// previously forked (i.e. hasn't run yet)
switch_asm_fork:
    // at this point, the stack doesn't match what we would
    // expect in a function call (i.e. the parameters prev, next, last
    // are not on the stack) : it was artificially set up 
    // according to intr_stack_t

    // do some scheduling stuff... 
    // (we will not return from switch_asm)
    // if we need, last is still in %eax 

    // return from the fork interrupt (as the new process)
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


