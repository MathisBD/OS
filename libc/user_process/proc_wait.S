#include "syscall_internal.h"


.global proc_wait
proc_wait:
    mov $SC_PROC_WAIT, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    