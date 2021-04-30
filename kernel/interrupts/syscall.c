#include "interrupts/syscall.h"
#include "threads/process.h"
#include <panic.h>
#include <stdio.h>

uint32_t get_syscall_arg(intr_frame_t* frame, uint32_t arg)
{
    switch(arg) {
    case 0: return frame->eax;
    case 1: return frame->ebx;
    case 2: return frame->ecx;
    case 3: return frame->edx;
    case 4: return frame->edi;
    case 5: return frame->esi;
    default: panic("invalid syscall arg index\n"); return 0;
    }
}


void handle_syscall(intr_frame_t* frame) 
{
    switch (frame->eax) {
    case SC_PROC_FORK:
    {
		do_proc_fork(frame);
		return;
    }
    case SC_PROC_EXEC:
    {
        do_proc_exec(frame);
        return;
    }
    default:
        printf("Unknown system call !\nsyscall number=0x%x\n", frame->eax);
        while(1);
    }
}
