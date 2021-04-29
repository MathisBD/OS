#pragma once

// system calls take their arguments in registers, 
// in the following order :
// eax, ebx, ecx, edx, edi, esi
// the first argument is always the system call number
// the return value is put in eax

// syscall interrupt number
#define SC_INTR 0x80

#define SC_PROC_FORK        1
#define SC_PROC_EXEC        2
#define SC_PROC_EXIT        3
#define SC_PROC_WAIT        4

#define SC_THREAD_CREATE    5
#define SC_THREAD_YIELD     6
#define SC_THREAD_JOIN      7
#define SC_THREAD_EXIT      8


#define SC_ARG_0    %eax
#define SC_ARG_1    %ebx
#define SC_ARG_2    %ecx
#define SC_ARG_3    %edx
#define SC_ARG_4    %edi
#define SC_ARG_5    %esi
