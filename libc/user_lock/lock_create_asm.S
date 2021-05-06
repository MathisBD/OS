
#ifdef __is_libc
#include "syscall_internal.h"
.global lock_create
lock_create:
    mov $SC_LOCK_CREATE, SC_ARG_0
    int $SC_INTR
    ret 
#endif 
