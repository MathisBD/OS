#include "syscall_internal.h"

.global proc_fork
proc_fork:
    mov $SC_PROC_FORK, SC_ARG_0
    int $SC_INTR
    ret 