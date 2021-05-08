#include "syscall_internal.h"


.global thread_join
thread_join:
    mov $SC_THREAD_JOIN, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    