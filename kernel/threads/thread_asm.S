#include "memory/constants.h"

// arguments :
// 1) prev : thread_t* 
// 2) next : thread_t*
// 3) esp_ofs : offset in bytes of the esp field in thread_t
// 4) proc_ofs : offset in bytes of the process field in thread_t 
// 5) pt_ofs : offset in bytes of the page_table field in process_t
.global thread_switch_asm
thread_switch_asm:
    // gcc already saved nearly all registers on stack
    // when calling this function.
    // we just have to save ebx and ebp.
    push %ebp
    mov %esp, %ebp
    push %ebx

    // get the arguments
    mov 8(%ebp), %esi  // prev
    mov 12(%ebp), %edi // next
    mov 16(%ebp), %ecx // esp_ofs
    mov 20(%ebp), %ebx // proc_ofs  
    mov 24(%ebp), %edx // pt_ofs

    // switch stacks
    // prev->esp = %esp
    mov %esp, (%esi, %ecx)
    // %esp = next->esp   
    mov (%edi, %ecx), %esp 

    mov (%esi, %ebx), %ebx // get the new process
    /*// switch in the new page directory.
    // the code of this function (thread_switch) and
    // the stack we are using are all in kernel space,
    // and the page directories all map the kernel space
    // to the same place, so we can safely do the switch.
    mov (%ebx, %edx), %ebx // get the new page table address
    sub $V_KERNEL_START, %ebx // compute the physical address
    mov %ebx, %cr3 // load the page table*/

    // restore ebx and ebp
    pop %ebx 
    pop %ebp

    // This will jump to the address on the stack.
    // If this is the first time the thread is running,
    // this will be a stub.
    // Otherwise the calling function will restore all 
    // the registers pushed by gcc.
    ret

