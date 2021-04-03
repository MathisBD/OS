#pragma once
#include "scheduler/process.h"

// 0 : highest priority
// PROC_PRIO_CNT-1 : lowest priority
#define PROC_PRIO_CNT 50
// in milliseconds
#define DEFAULT_PROC_TIME 100
#define STATUS_RUN  1

extern proc_desc_t* curr_proc;

void init_scheduler();
void sched_new_thread(proc_desc_t* thread);
// delta_time : time in milliseconds since the last time
// this function was called
void scheduler_tick(float delta_time);
void schedule();
// returns the pid of the current process
pid_t get_pid();

