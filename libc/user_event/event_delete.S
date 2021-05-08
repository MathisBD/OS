#include "syscall_internal.h"


.global event_delete
event_delete:
    mov $SC_EVENT_DELETE, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    