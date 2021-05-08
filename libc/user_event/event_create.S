#include "syscall_internal.h"


.global event_create
event_create:
    mov $SC_EVENT_CREATE, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    