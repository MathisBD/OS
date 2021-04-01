#include "__syscall_internal.h"

.global exit 
exit:
    mov $SC_EXIT, SC_ARG_0 
    mov 4(%esp), SC_ARG_1 
    int $SC_INTR
    ret

    