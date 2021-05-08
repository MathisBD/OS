#include "syscall_internal.h"


.global event_signal
event_signal:
    mov $SC_EVENT_SIGNAL, SC_ARG_0
    mov 4(%esp), SC_ARG_1
    int $SC_INTR
    ret 
    