#include "syscall_internal.h"

.global file_type
file_type:
    mov $SC_FILE_TYPE, SC_ARG_0 
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
