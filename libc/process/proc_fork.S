#include "syscall_internal.h"

.global proc_fork
proc_fork:
    mov $SC_PROC_FORK, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 