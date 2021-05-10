#include "syscall_internal.h"

.global close
close:
    mov $SC_CLOSE, SC_ARG_0
    mov 4(%esp), SC_ARG_1 
    int $SC_INTR 
    ret 

