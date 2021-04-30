#include "syscall_internal.h"

.global proc_exec
proc_exec:
    mov $SC_PROC_EXEC, SC_ARG_0 
    mov 4(%esp), SC_ARG_1 
    int $SC_INTR 
    ret
    