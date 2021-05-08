#include "syscall_internal.h"


.global thread_exit
thread_exit:
    mov $SC_THREAD_EXIT, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    