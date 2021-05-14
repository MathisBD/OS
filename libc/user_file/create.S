#include "syscall_internal.h"

.global create
create:
    mov $SC_CREATE, SC_ARG_0 
    mov 4(%esp), SC_ARG_1
    mov 8(%esp), SC_ARG_2
    int $SC_INTR 
    ret 
