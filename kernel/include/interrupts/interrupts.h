#pragma once 
#include <stdint.h>
#include <stdbool.h>


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

// returns the old state of interrupts.
// true means they were enabled.
bool set_interrupt_flag(bool enable);
bool get_interrupt_flag();
void handle_interrupt(intr_frame_t* frame);