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
#include "tables/gdt.h"


#define MAX_PROC_COUNT  1000

// protects global process data such as next_pid
static queuelock_t* proc_lock;

static pid_t next_pid = 0;
pid_t new_pid()
{
    queuelock_acquire(proc_lock);
    if (next_pid >= MAX_PROC_COUNT) {
        panic("max process count reached\n");
    }
    pid_t tmp = next_pid;
    next_pid++;
    queuelock_release(proc_lock);
    return tmp;
}

void init_process()
{
    proc_lock = queuelock_create();

    // create the kernel process
    process_t* proc = kmalloc(sizeof(process_t));
    proc->pid = new_pid();
    proc->lock = queuelock_create();
    proc->parent = 0;
    proc->children = list_create();
    proc->state = PROC_ALIVE;

    thread_t* thread = curr_thread();
    thread->process = proc;
    proc->threads = list_create();
    list_add_front(proc->threads, thread);

    proc->page_table = kernel_page_table();
}

static process_t* copy_process(process_t* original)
{
    queuelock_acquire(original->lock);

    // create a new process
    process_t* copy = kmalloc(sizeof(process_t));
    copy->pid = new_pid();
    copy->lock = queuelock_create();
    // process relationships
    copy->parent = original;
    copy->children = list_create();
    list_add_back(original->children, copy);
    copy->children = list_create();

    // page table (has to be aligned on 4K)
    copy->page_table = kmalloc_aligned(PAGE_TABLE_SIZE * sizeof(uint32_t), 4096);
    copy_address_space(copy->page_table, original->page_table);

    queuelock_release(original->lock);
    return copy;
}

// assumes new_proc isn't visible by other threads yet
// (i.e. there is no need to lock it when modifying it).
static thread_t* copy_thread(thread_t* original, process_t* new_proc, intr_frame_t* frame)
{
    queuelock_acquire(original->lock);

    thread_t* copy = kmalloc(sizeof(thread_t));
    copy->tid = new_tid();
    copy->lock = queuelock_create();
    copy->on_finish = event_create(copy->lock);
    // dummy stack for the copied thread.
    // when thread_switch switches the copied thread in
    // and then returns, we want it to return to isr_common
    // (in idt_asm.S), just as if it had returned from the call
    // to handle_interrupt.
    copy->stack = kmalloc(KSTACK_SIZE);
    memcpy(copy->stack, original->stack, KSTACK_SIZE);
    int ofs = ((int)copy->stack) - ((int)original->stack);
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

    // return values :  the child gets 0,
    // the parent gets the child pid (nonzero).
    frame->eax = new_proc->pid;
    copy_frame->eax = 0; 

    // book-keeping
    list_add_front(new_proc->threads, copy);
    copy->process = new_proc;
}


void do_proc_fork(intr_frame_t* frame)
{
    queuelock_acquire(proc_lock);

    process_t* new_proc = copy_process(curr_thread()->process);
    thread_t* new_thread = copy_thread(curr_thread(), new_proc, frame);
    
    // everything is up and ready to run :
    // make the new process and thread visible to everyone.
    new_proc->state = PROC_ALIVE;
    sthread_create(new_thread);

    queuelock_release(proc_lock);
}

void do_proc_exec(intr_frame_t* frame)
{
    char* prog_name = get_syscall_arg(frame, 1);
    //free_user_pages();

    // load the user code/data
    uint32_t entry_addr;
    uint32_t user_stack_top;
    load_program(prog_name, &entry_addr, &user_stack_top);

    // assembly stub to jump
    extern void exec_jump_asm(uint32_t entry_addr, uint32_t user_stack_top);
    exec_jump_asm(entry_addr, user_stack_top);
}