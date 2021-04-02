#include "scheduler/process.h"
#include "interrupts/regs.h"
#include "memory/constants.h"
#include "memory/kheap.h"
#include "tables/gdt.h"
#include <panic.h>
#include "scheduler/do_syscall.h"
#include "scheduler/scheduler.h"
#include "scheduler/process.h"

// kernel/user thread
// kernel threads run only in kernel mode 
// and don't use any lower half memory
#define NEW_THREAD_FLAGS_KERNEL   1


void do_new_thread(intr_stack_t* pregs)
{
    int (*fn)(void*) = pregs->ebx;
    void* arg = pregs->ecx;
    void* stack = pregs->edx;
    uint32_t flags = pregs->edi;

    proc_desc_t* thread = kmalloc(sizeof(proc_desc_t));
    thread->pid = new_pid();
    // store the child pid in the parent eax
    pregs->eax = thread->pid;
    // setup the new stack : it has to mimic the stack
    // created by the assembly interrupt handler
    // (layout described by intr_stack_t)
    thread->kstack = kmalloc(KSTACK_SIZE);
    thread->ctx.esp0 = thread->kstack + KSTACK_SIZE;
    thread->ctx.esp = thread->ctx.esp0 - sizeof(intr_stack_t);
    intr_stack_t* thread_pregs = (intr_stack_t*)(thread->ctx.esp);
    
    if (!(flags & NEW_THREAD_FLAGS_KERNEL)) {
        panic("do_new_thread : user mode threads not implemented\n");
    }
    // copy segment selectors
    thread_pregs->cs = pregs->cs;
    thread_pregs->ds = pregs->ds;
    thread_pregs->es = pregs->es;
    thread_pregs->fs = pregs->fs;
    thread_pregs->gs = pregs->gs;
    // setup registers
    thread_pregs->ebp = 0;
    thread_pregs->edx = 0;
    thread_pregs->ecx = 0;
    thread_pregs->ebx = 0;
    thread_pregs->esi = 0;
    thread_pregs->edi = 0;
    thread_pregs->eax = arg;
    // copy eflags
    thread_pregs->eflags = pregs->eflags;
    // set eip so that the thread starts executing fn
    thread_pregs->eip = fn;
    // intr_num, error_code won't be used :
    // no need to set them

    // setup the instruction pointer : this is the
    // address of the code that will be executed when
    // the new process is first switched in
    extern void switch_asm_new_thread;
    thread->ctx.eip = &switch_asm_new_thread;

    // scheduler stuff
    sched_new_thread(thread);
}
