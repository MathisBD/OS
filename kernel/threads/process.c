#include "threads/process.h"
#include "threads/thread.h"
#include "threads/scheduler.h"
#include "interrupts/interrupts.h"
#include "interrupts/syscall.h"
#include <panic.h>
#include "memory/constants.h"
#include "memory/paging.h"
#include "memory/kheap.h"
#include <string.h>
#include <stdio.h>
#include "loader/loader.h"


#define MAX_PROC_COUNT  1000

static pid_t next_pid = 0;
pid_t new_pid()
{
    if (next_pid >= MAX_PROC_COUNT) {
        panic("max process count reached\n");
    }
    pid_t tmp = next_pid;
    next_pid++;
    return tmp;
}

void init_process()
{
    // create the kernel process
    process_t* proc = kmalloc(sizeof(process_t));
    proc->pid = new_pid();
    proc->parent = 0;
    proc->children = list_create();
    proc->state = PROC_ALIVE;
    
    thread_t* thread = curr_thread();
    thread->process = proc;
    proc->threads = list_create();
    list_add_front(proc->threads, thread);

    proc->page_table = kernel_page_table();
}


void do_proc_fork(intr_frame_t* frame)
{
    thread_t* curr = curr_thread();

    // create a new process
    process_t* proc = kmalloc(sizeof(process_t));
    proc->pid = new_pid();
    // process relationsships
    proc->parent = curr->process;
    proc->children = list_create();
    list_add_back(curr->process->children, proc);
    // page table (has to be aligned on 4K)
    proc->page_table = kmalloc_aligned(PAGE_TABLE_SIZE * sizeof(uint32_t), 4096);
    copy_address_space(proc->page_table, curr->process->page_table);

    // copy the thread
    thread_t* copy = kmalloc(sizeof(thread_t));
    copy->tid = new_tid();
    // dummy stack for the copied thread.
    // when thread_switch switches the copied thread in
    // and then returns, we want it to return to isr_common
    // (in idt_asm.S), just as if it had returned from the call
    // to handle_interrupt.
    copy->stack = kmalloc(KSTACK_SIZE);
    memcpy(copy->stack, curr->stack, KSTACK_SIZE);
    int ofs = ((int)copy->stack) - ((int)curr->stack);
    intr_frame_t* copy_frame = ((void*)frame) + ofs;
    // remember stacks grow towards the bottom,
    // and copy_frame points to the lowest address of the struct
    copy->esp = copy_frame;
    // handle_interrupt argument (see idt_asm.S)
    // (if we wanted this argument to be valid,
    // we could do *(copy->esp) = copy_frame)
    copy->esp--;
    // return address the original thread pushed for 
    // handle_interrupt. this will now become the return
    // address for thread_switch.
    copy->esp--;
    // dummy ebx and ebp registers for thread_switch
    copy->esp--;
    copy->esp--;

    // return values. the child gets 0,
    // the parent gets the child pid (nonzero).
    frame->eax = proc->pid;
    copy_frame->eax = 0; 

    // book-keeping
    proc->threads = list_create();
    list_add_front(proc->threads, copy);

    // everything is up and ready to run !
    proc->state = PROC_ALIVE;
    disable_interrupts();
    sthread_create(copy);
    enable_interrupts();
}

void do_proc_exec(intr_frame_t* frame)
{
    // load the user code/data
    char* prog_name = get_syscall_arg(frame, 1);
    free_user_pages();
    void* new_stack = load_program(prog_name);
    
}