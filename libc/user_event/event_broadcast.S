#include "syscall_internal.h"


.global event_broadcast
event_broadcast:
    mov $SC_EVENT_BROADCAST, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    