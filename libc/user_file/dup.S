#include "syscall_internal.h"

.global dup
dup:
    mov $SC_DUP, SC_ARG_0
    mov 4(%esp), SC_ARG_1 
    int $SC_INTR 
    ret 
