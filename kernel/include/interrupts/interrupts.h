#pragma once 
#include "interrupts/regs.h"


/*// system calls takes their arguments in registers, 
// in the following order :
// eax, ebx, ecx, edx, edi, esi
// the first argument is always the system call number
// the return value is put in eax

#define SC_NEW_THREAD       1
#define SC_NEW_PROCESS      2
#define SC_EXIT             3*/


void enable_interrupts();
void disable_interrupts();
void handle_interrupt(intr_stack_t* pregs);