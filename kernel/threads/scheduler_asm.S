#include "memory/constants.h"

// arguments :
// 1) prev : thread_t* 
// 2) next : thread_t*
// 3) esp_ofs : offset in bytes of the esp field in thread_switch 
// 4) page_table : physical address of the new page table
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
    mov 20(%ebp), %ebx // page_table_address

    // switch stacks
    // prev->esp = %esp
    mov %esp, (%esi, %ecx)
    // %esp = next->esp   
    mov (%edi, %ecx), %esp 

    // switch in the new page directory.
    // the code of this function (thread_switch) and
    // the stack we are using are all in kernel space,
    // and the page directories all map the kernel space
    // to the same place, so we can safely do the switch.
    mov %ebx, %cr3

    // restore ebx and ebp.
    // the order matters here (it is used in process.c for example)
    pop %ebx 
    pop %ebp

    // This will jump to the address on the stack.
    // If this is the first time the thread is running,
    // this will be a stub.
    // Otherwise the calling function will restore all 
    // the registers pushed by gcc.
    ret²

