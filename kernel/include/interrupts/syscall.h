#pragma once
#include "interrupts/interrupts.h"

// system calls take their arguments in registers, 
// in the following order :
// eax, ebx, ecx, edx, edi, esi
// the first argument is always the system call number
// the return value is put in eax

// syscall interrupt number
#define SC_INTR 0x80

#define SC_PROC_FORK        1
#define SC_PROC_EXEC        2
#define SC_PROC_EXIT        3
#define SC_PROC_WAIT        4

#define SC_THREAD_CREATE    5
#define SC_THREAD_YIELD     6
#define SC_THREAD_JOIN      7
#define SC_THREAD_EXIT      8


// arg 0 is the syscall number
uint32_t get_syscall_arg(intr_frame_t* frame, uint32_t arg);

void handle_syscall(intr_frame_t* frame);