#include "__syscall_internal.h"


.global new_thread 
new_thread:
    // bundle the function pointer and the arg together
#ifdef __is_libk
    .extern kmalloc
    push $8 
    call kmalloc
    add $4, %esp
#endif
#ifdef __is_libc
    // use libc malloc instead
#endif
    // function
    mov 4(%esp), %ecx 
    mov %ecx, (%eax)   
    // arg 
    mov 8(%esp), %ecx 
    mov %ecx, 4(%eax) 

    push %eax 
    mov $SC_NEW_THREAD, SC_ARG_0
    mov $thread_wrapper, SC_ARG_1 // wrapper function
    pop SC_ARG_2 // bundle arg 
    mov 12(%esp), SC_ARG_3 // stack 
    mov 16(%esp), SC_ARG_4 // flags
    int $SC_INTR
    ret 


.extern exit 
// this is the first code a newly created thread executes.
// assumes the argument is in eax.
thread_wrapper:
    mov (%eax), %ebx // function
    mov 4(%eax), %ecx // arg 
    push %ecx 
    call *%ebx
    // exit from the process
    // with the error code the function just gave us 
    push %eax 
    call exit

