#include "syscall_internal.h"

.global get_size
get_size:
    mov $SC_GET_SIZE, SC_ARG_0 
    mov 4(%esp), SC_ARG_1
    int $SC_INTR 
    ret 
