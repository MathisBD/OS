#pragma once 
#include <stdint.h>


/*// system calls takes their arguments in registers, 
// in the following order :
// eax, ebx, ecx, edx, edi, esi
// the first argument is always the system call number
// the return value is put in eax

#define SC_NEW_THREAD       1
#define SC_NEW_PROCESS      2
#define SC_EXIT             3*/


// maps the contents of the stack when handling an interrupt.
// some is pushed by the cpu, some by the assembly wrapper
// before control is passed to C interrupt handling functions
typedef struct {
	// user data segment selectors
	uint32_t gs, fs, es, ds;
	// user registers
	uint32_t ebp, edx, ecx, ebx, eax, esi, edi;
	// pushed by the isrxx
	uint32_t intr_num, error_code;
	// pushed by the cpu
	uint32_t eip, cs, eflags;
} __attribute__((packed)) intr_frame_t;


void enable_interrupts();
void disable_interrupts();
void handle_interrupt(intr_frame_t* pregs);