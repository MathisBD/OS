#include "syscall_internal.h"

.global chdir
chdir:
    mov $SC_CHDIR, SC_ARG_0 
    mov 4(%esp), SC_ARG_1
    int $SC_INTR 
    ret 
    