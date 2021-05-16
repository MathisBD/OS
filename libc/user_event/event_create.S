#include "syscall_internal.h"


.global event_create
event_create:
    mov $SC_EVENT_CREATE, SC_ARG_0
    int $SC_INTR
    ret 
    