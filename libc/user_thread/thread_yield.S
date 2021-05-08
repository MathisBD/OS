#include "syscall_internal.h"


.global thread_yield
thread_yield:
    mov $SC_THREAD_YIELD, SC_ARG_0
    int $SC_INTR
    ret 
    