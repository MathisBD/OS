#include "syscall_internal.h"

.global proc_data_size
proc_data_size:
    mov $SC_PROC_DATA_SIZE, SC_ARG_0 
    int $SC_INTR 
    ret 
    