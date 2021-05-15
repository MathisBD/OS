#pragma once
#include "interrupts/interrupts.h"

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

#define SC_QL_CREATE        9
#define SC_QL_DELETE        10
#define SC_QL_ACQUIRE       11
#define SC_QL_RELEASE       12

#define SC_OPEN             13
#define SC_CLOSE            14
#define SC_READ             15
#define SC_WRITE            16
#define SC_PIPE             17
#define SC_SEEK             18

#define SC_EVENT_CREATE     20
#define SC_EVENT_DELETE     21
#define SC_EVENT_WAIT       22
#define SC_EVENT_SIGNAL     23
#define SC_EVENT_BROADCAST  24

#define SC_PROC_DATA_SIZE   25
#define SC_PROC_STACK_SIZE  26

// file/dir manipulation
#define SC_CREATE           27
#define SC_REMOVE           28
#define SC_GET_SIZE         29
#define SC_RESIZE           30
#define SC_LIST_DIR         31
#define SC_DUP              32
#define SC_DUP2             33
#define SC_CHDIR            34
#define SC_GETCWD           35
#define SC_FILE_TYPE        36


// arg 0 is the syscall number
uint32_t get_syscall_arg(intr_frame_t* frame, uint32_t arg);
void set_syscall_arg(intr_frame_t* frame, uint32_t arg, uint32_t value);
void handle_syscall(intr_frame_t* frame);