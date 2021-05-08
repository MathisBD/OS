#include "syscall_internal.h"


.global lock_release
lock_release:
    mov $SC_QL_RELEASE, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    