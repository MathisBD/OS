#pragma once 
#include "interrupts/regs.h"


#define SYSCALL_FORK    18

void handle_interrupt(intr_stack_t* pregs);