#include "syscall_internal.h"

.global proc_stack_size
proc_stack_size:
    mov $SC_PROC_STACK_SIZE, SC_ARG_0 
    int $SC_INTR 
    ret 
    