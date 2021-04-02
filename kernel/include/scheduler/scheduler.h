#pragma once
#include "scheduler/process.h"


#define STATUS_RUN  1

extern proc_desc_t* curr_proc;

void init_scheduler();
void sched_new_thread(proc_desc_t* thread);
void schedule();
// returns the pid of the current process
pid_t get_pid();

