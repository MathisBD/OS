#include "syscall_internal.h"


.global lock_acquire
lock_acquire:
    mov $SC_QL_ACQUIRE, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    