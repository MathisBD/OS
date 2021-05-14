#include "syscall_internal.h"

.global resize
resize:
    mov $SC_RESIZE, SC_ARG_0 
    mov 4(%esp), SC_ARG_1
    mov 8(%esp), SC_ARG_2
    int $SC_INTR 
    ret 
