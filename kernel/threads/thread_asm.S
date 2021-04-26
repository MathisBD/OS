// arguments :
// 1) prev : thread_t* 
// 2) next : thread_t*
// 3) esp_ofs : offset in bytes of the esp field in thread_t
.global thread_switch_asm
thread_switch_asm:
    // gcc already saved nearly all registers on stack
    // when calling this function.
    // we just have to save ebx and ebp.
    push %ebp
    mov %esp, %ebp
    push %ebx

    .extern sthread_switch
    push 12(%ebp)   // next
    push 8(%ebp)    // prev
    call sthread_switch
    add $8, %esp

    // get the arguments
    mov 8(%ebp), %esi  // prev
    mov 12(%ebp), %edi // next
    mov 16(%ebp), %ecx // esp_ofs

    // switch stacks
    // prev->esp = %esp
    mov %esp, (%esi, %ecx)
    // %esp = next->esp   
    mov (%edi, %ecx), %esp 

    // restore ebx and ebp
    pop %ebx 
    pop %ebp
    // This will jump to the address on the stack.
    // If this is the first time the thread is running,
    // this will be a stub.
    // Otherwise the calling function will restore all 
    // the registers pushed by gcc.
    ret

