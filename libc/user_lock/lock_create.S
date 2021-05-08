#include "syscall_internal.h"


.global lock_create
lock_create:
    mov $SC_QL_CREATE, SC_ARG_0
    int $SC_INTR
    ret 
    