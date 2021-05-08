#include "syscall_internal.h"


.global proc
proc_exit:
    mov $SC_PROC_EXIT, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    