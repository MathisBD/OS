#include "syscall_internal.h"


.global event_wait
event_wait:
    mov $SC_EVENT_WAIT, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    