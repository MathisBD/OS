#pragma once
#include "scheduler/process.h"


#define STATUS_RUN  1

extern proc_desc_t* curr_proc;

void schedule();
void switch_proc(proc_desc_t* prev, proc_desc_t* next);

// returns the pid of the current process
pid_t get_pid();