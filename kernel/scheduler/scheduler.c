#include "scheduler/scheduler.h"


// processes that are running
ll_part_t run_head;


void switch_proc(proc_desc_t* prev, proc_desc_t* next);