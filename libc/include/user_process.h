#pragma once
#include <stdint.h>


typedef uint32_t pid_t;

pid_t proc_fork();
void proc_exec(char* prog_name);
void proc_exit(int exit_code);
int proc_wait(pid_t pid);
