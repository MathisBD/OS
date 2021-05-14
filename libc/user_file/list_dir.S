#include "syscall_internal.h"

.global list_dir
list_dir:
    mov $SC_LIST_DIR, SC_ARG_0 
    mov 4(%esp), SC_ARG_1
    mov 8(%esp), SC_ARG_2 
    mov 12(%esp), SC_ARG_3
    int $SC_INTR 
    ret 
