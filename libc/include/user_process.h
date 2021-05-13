#pragma once
#include <stdint.h>


typedef uint32_t pid_t;


uint32_t proc_stack_size();
uint32_t proc_data_size();

pid_t proc_fork();
void proc_exec(char* prog_name, int argc, char** argv);
void proc_exit(int exit_code);
int proc_wait(pid_t pid);
