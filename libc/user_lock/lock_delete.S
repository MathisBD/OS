#include "syscall_internal.h"


.global lock_delete
lock_delete:
    mov $SC_QL_DELETE, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    