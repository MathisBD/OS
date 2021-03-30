#include "scheduler/process.h"
#include "memory/heap.h"
#include "memory/constants.h"
#include <string.h>
#include "interrupts/regs.h"


// pid 0 is reserved for the init process
uint32_t curr_pid = 1;

// hashtable : pid -> proc descriptor


pid_t new_pid()
{
    pid_t tmp = curr_pid;
    curr_pid++;
    return tmp;
}


proc_desc_t* fork(proc_desc_t* src, uint32_t flags, intr_stack_t* pregs)
{
    proc_desc_t* dest = malloc(sizeof(proc_desc_t));
    // setup the new stack : it has to mimic the stack
    // created by the assembly interrupt handler
    // (layout described by intr_stack_t) when handling the 
    // system call 'fork' of src
    dest->kstack = malloc(KSTACK_SIZE);
    dest->ctx.esp0 = dest->kstack + KSTACK_SIZE;
    memcpy(
        dest->ctx.esp0 - sizeof(intr_stack_t),
        pregs,
        sizeof(intr_stack_t)
    );
    dest->ctx.esp = dest->ctx.esp0 - sizeof(intr_stack_t);
    // we set eax to 0 for the new process 
    intr_stack_t* new_pregs = (intr_stack_t*)(dest->ctx.esp);
    new_pregs->eax = 0; 
    // setup the instruction pointer : this is the
    // address of the code that will be executed when
    // the new process is first switched in
    extern void switch_asm_fork;
    dest->ctx.eip = &switch_asm_fork;

    return dest;
}


void free_proc(proc_desc_t* proc)
{
    free(proc->kstack);
    free(proc);
}

void create_init_proc(proc_desc_t** proc)
{
    extern uint32_t init_stack_bottom; // see boot.S
    extern uint32_t get_esp(); // see process_asm.S

    *proc = malloc(sizeof(proc_desc_t));
    (*proc)->kstack = &init_stack_bottom;
    (*proc)->ctx.esp0 = (*proc)->kstack + KSTACK_SIZE;
    //(*proc)->ctx.esp = get_esp();

    (*proc)->pid = 0;
}
