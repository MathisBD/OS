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

static process_t** proc_array;
static pid_t next_pid = 0;
// assumes the process isn't visible to anyone yet
// i.e. there is no need to lock it.
static pid_t register_process(process_t* proc)
{
    kql_acquire(proc_lock);
    if (next_pid >= MAX_PROC_COUNT) {
        panic("max process count reached\n");
    }
    pid_t pid = next_pid;
    next_pid++;
    proc->pid = pid;
    proc_array[pid] = proc;
    kql_release(proc_lock);
    return pid;
}

static process_t* get_process(pid_t pid)
{
    kql_acquire(proc_lock);
    process_t* proc = proc_array[pid];
    kql_release(proc_lock);
    return proc;
}


void init_process()
{
    proc_lock = kql_create();

    proc_array = kmalloc(MAX_PROC_COUNT * sizeof(process_t*));
    memset(proc_array, 0, MAX_PROC_COUNT * sizeof(process_t*));

    // create the kernel process
    process_t* proc = kmalloc(sizeof(process_t));
    proc->lock = kql_create();
    proc->parent = 0;
    proc->children = list_create();
    // scheduler state
    proc->state = PROC_ALIVE;
    proc->on_finish = kevent_create(proc->lock);
    // process resources
    proc->locks = vect_create();
    proc->events = vect_create();
    proc->file_descrs = vect_create();
    proc->cwd = "/";
    // data and stack size don't have any sense for a kernel only process
    proc->data_size = 0;
    proc->stack_size = 0;
    proc->page_table = kernel_page_table();

    thread_t* thread = curr_thread();
    thread->process = proc;
    proc->threads = list_create();
    list_add_front(proc->threads, thread);

    register_process(proc);
}


static uint32_t add_resource(vect_t* vect, void* res)
{
    for (uint32_t i = 0; i < vect->size; i++) {
        if (vect_get(vect, i) == 0) {
            vect_set(vect, i, res);
            return i;
        }
    }
    vect_append(vect, res);
    return vect->size - 1;
}


uint32_t proc_add_lock(process_t* proc, queuelock_t* lock)
{
    kql_acquire(proc->lock);
    uint32_t id = add_resource(proc->locks, lock);
    kql_release(proc->lock);
    return id;
}

queuelock_t* proc_get_lock(process_t* proc, uint32_t id)
{
    kql_acquire(proc->lock);
    queuelock_t* lock = vect_get(proc->locks, id);
    kql_release(proc->lock);
    return lock;
}

queuelock_t* proc_remove_lock(process_t* proc, uint32_t id)
{
    kql_acquire(proc->lock);
    queuelock_t* lock = vect_set(proc->locks, id, 0);
    kql_release(proc->lock);
    return lock;
}

uint32_t proc_add_event(process_t* proc, event_t* event)
{
    kql_acquire(proc->lock);
    uint32_t id = add_resource(proc->events, event);
    kql_release(proc->lock);
    return id;
}

event_t* proc_get_event(process_t* proc, uint32_t id)
{
    kql_acquire(proc->lock);
    event_t* event = vect_get(proc->events, id);
    kql_release(proc->lock);
    return event;
}

event_t* proc_remove_event(process_t* proc, uint32_t id)
{
    kql_acquire(proc->lock);
    event_t* event = vect_set(proc->events, id, 0);
    kql_release(proc->lock);
    return event;
}

uint32_t proc_add_fd(process_t* proc, file_descr_t* fd)
{
    kql_acquire(proc->lock);
    uint32_t id = add_resource(proc->file_descrs, fd);
    kql_release(proc->lock);
    return id;
}

file_descr_t* proc_get_fd(process_t* proc, uint32_t id)
{
    kql_acquire(proc->lock);
    file_descr_t* fd = vect_get(proc->file_descrs, id);
    kql_release(proc->lock);
    return fd;
}

file_descr_t* proc_remove_fd(process_t* proc, uint32_t id)
{
    kql_acquire(proc->lock);
    file_descr_t* fd = vect_set(proc->file_descrs, id, 0);
    kql_release(proc->lock);
    return fd;
}

void copy_locks(process_t* copy, process_t* original)
{
    copy->locks = vect_create();
    vect_grow(copy->locks, original->locks->size);
    for (uint32_t i = 0; i < original->locks->size; i++) {
        queuelock_t* lock = vect_get(original->locks, i);
        if (!list_empty(lock->waiting)) {
            // shouldn't happen since there is only one thread in the process
            panic("copy_locks");
        }
        queuelock_t* new_lock = kql_create();
        new_lock->value = lock->value;
        vect_append(copy->locks, lock);
    }
}

void copy_events(process_t* copy, process_t* original)
{
    copy->events = vect_create();
    vect_grow(copy->events, original->events->size);
    for (uint32_t i = 0; i < original->events->size; i++) {
        event_t* ev = vect_get(original->events, i);
        if (!list_empty(ev->waiting)) {
            // shouldn't happen since there is only one thread in the process
            panic("copy_events");
        }
        event_t* new_ev = kevent_create();
        vect_append(copy->events, new_ev);
    }
}


void copy_file_descrs(process_t* copy, process_t* original)
{
    copy->file_descrs = vect_create();
    vect_grow(copy->file_descrs, original->file_descrs->size);
    for (uint32_t i = 0; i < original->file_descrs->size; i++) {
        file_descr_t* fd = vect_get(original->file_descrs, i);
        fd = fd_copy(fd);
        vect_append(copy->file_descrs, fd);
    }
}


static process_t* copy_process(process_t* original)
{
    kql_acquire(original->lock);

    // create a new process
    process_t* copy = kmalloc(sizeof(process_t));
    copy->lock = kql_create();
    copy->on_finish = kevent_create(copy->lock);
    // process relationships
    copy->parent = original;
    copy->children = list_create();
    list_add_back(original->children, copy);
    copy->threads = list_create();

    // page table (has to be aligned on 4K)
    copy->page_table = kmalloc_aligned(PAGE_TABLE_SIZE * sizeof(uint32_t), 4096);
    copy_address_space(copy->page_table, original->page_table);

    // process resources
    copy_locks(copy, original);
    copy_events(copy, original);
    copy_file_descrs(copy, original);
    // cwd is inherited
    uint32_t len = strlen(original->cwd) + 1;
    copy->cwd = kmalloc(len);
    memcpy(copy->cwd, original->cwd, len);

    register_process(copy);

    kql_release(original->lock);
    return copy;
}

// assumes new_proc isn't visible by other threads yet
// (i.e. there is no need to lock it when modifying it).
static thread_t* copy_thread(thread_t* original, process_t* new_proc, intr_frame_t* frame)
{
    kql_acquire(original->lock);

    thread_t* copy = kmalloc(sizeof(thread_t));
    copy->lock = kql_create();
    copy->on_finish = kevent_create(copy->lock);
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
    // align the stack exactly as is done in mCallHandler (idt_asm.S)
    copy->esp = ((uint32_t)(copy->esp)) & (-16);
    copy->esp -= 3;
    // handle_interrupt argument (see idt_asm.S)
    // (if we wanted this argument to be valid,
    // we could do *(copy->esp) = copy_frame)
    copy->esp--;
    // return address the original thread pushed for 
    // handle_interrupt. this will now become the return
    // address for the stub.
    // notice the stack is aligned for the stub, because the stub
    // replaces the handle_interrupt() function.
    copy->esp--;
    // no need to align the stack for thread_switch_asm
    // return address of thread_switch_asm : it will jump to a stub
    // (that will unlock the scheduler).
    copy->esp--; *(copy->esp) = forked_thread_stub;
    // ebx and ebp registers for thread_switch_asm
    copy->esp--; *(copy->esp) = copy_frame; // ebp
    copy->esp--; // dummy ebx

    // return values : the child gets 0,
    // the parent gets the child pid (nonzero).
    frame->eax = new_proc->pid;
    copy_frame->eax = 0; 

    // book-keeping
    list_add_front(new_proc->threads, copy);
    copy->process = new_proc;

    register_thread(copy);

    kql_release(original->lock);
    return copy;
}


static bool is_singleton(list_t* l)
{
    return l->first != 0 && l->first == l->last;
}

void do_proc_fork(intr_frame_t* frame)
{
    kql_acquire(proc_lock);    
    
    if (!is_singleton(curr_process()->threads)) {
        panic("can't call fork() from a process that has more than one thread");
    }

    process_t* new_proc = copy_process(curr_process());
    thread_t* new_thread = copy_thread(curr_thread(), new_proc, frame);

    // everything is up and ready to run :
    // make the new process and thread visible to everyone.
    new_proc->state = PROC_ALIVE;
    sthread_create(new_thread);

    kql_release(proc_lock);
}


static uint32_t align(value, a)
{
    if (value % a != 0) {
        return (value / a) * (a+1);
    }
    return value;
}

void kproc_exec(char* prog, int argc, char** argv)
{
    process_t* proc = curr_process();
    kql_acquire(proc->lock);

    if (!is_singleton(proc->threads)) {
        panic("can't call exec() from a process that has more than one thread");
    }

    // copy args to kernel space
    char** kargv = kmalloc(argc * sizeof(char*));
    for (uint32_t i = 0; i < argc; i++) {
        uint32_t len = strlen(argv[i]) + 1;
        kargv[i] = kmalloc(len);
        memcpy(kargv[i], argv[i], len);
    }

    // load the user code/data
    //free_user_pages();
    uint32_t entry_addr;
    uint32_t user_stack_top;
    uint32_t data_size;
    uint32_t stack_size;
    load_program(prog, &entry_addr, &user_stack_top, &data_size, &stack_size);

    // copy args to the new user space
    uint32_t addr = data_size;
    uint32_t uargv = addr;
    addr += argc * sizeof(char*);
    for (uint32_t i = 0; i < argc; i++) {
        ((uint32_t*)uargv)[i] = addr;
        uint32_t size = strlen(kargv[i]) + 1;
        memcpy(addr, kargv[i], size);
        addr += size;
    }
    proc->stack_size = stack_size;
    proc->data_size = addr; // skip argv

    // prepare initial user stack
    uint32_t* esp = (uint32_t*)user_stack_top;
    esp -= 2; // stack alignment
    esp--; *esp = uargv;
    esp--; *esp = argc;
    esp--; // dummy return address

    kql_release(proc->lock);

    // assembly stub to jump
    extern void exec_jump_asm(
        uint32_t entry_addr, 
        uint32_t user_esp);
    exec_jump_asm(entry_addr, esp);
}

static void move_to_root(process_t* proc)
{
    process_t* root = get_process(0);
    kql_acquire(proc->lock);
    kql_acquire(root->lock);
    proc->parent = root;
    list_add_back(root->children, proc);
    kql_release(root->lock);
    kql_release(proc->lock);
}

void kproc_exit(int code)
{
    process_t* proc = curr_process();
    kql_acquire(proc->lock);
    if (proc->pid == 0) {
        panic("process 0 can't exit (it has to wait() on its children)");
    }
    if (!is_singleton(proc->threads)) {
        panic("can't call exit() from a process that has more than one thread");
    }

    proc->exit_code = code;
    kevent_broadcast(proc->on_finish);
    // move all children to process 0
    for (list_node_t* node = proc->children->first; node != 0; node = node->next) {
        process_t* proc = node->contents;
        move_to_root(proc);
    }

    sched_finish_proc_and_release(proc->lock);
    panic("executing code in finished process !");
}

// the process is in DEAD state and its threads
// are in FINISHED state.
static void delete_proc(process_t* proc)
{
    kql_acquire(proc_lock);

    kql_acquire(proc->lock);
    proc_array[proc->pid] = 0;
    // delete the process' threads
    for (list_node_t* node = proc->threads->first; node != 0; node = node->next) {
        delete_thread(node->contents);
    }
    list_delete(proc->threads);
    list_delete(proc->children);
    // free the user pages
    //free_user_pages();
    // close every file descriptor
    for (uint32_t i = 0; i < proc->file_descrs->size; i++) {
        file_descr_t* fd = vect_get(proc->file_descrs, i);
        if (fd != 0) {
            //kclose(fd);
        }
    }
    vect_delete(proc->file_descrs);
    // delete events 
    for (uint32_t i = 0; i < proc->events->size; i++) {
        event_t* event = vect_get(proc->events, i);
        if (event != 0) {
            kevent_delete(event);
        }
    }
    vect_delete(proc->events);
    // delete locks
    for (uint32_t i = 0; i < proc->locks->size; i++) {
        queuelock_t* lock = vect_get(proc->locks, i);
        if (lock != 0) {
            kql_delete(lock);
        }
    }
    vect_delete(proc->locks);

    kfree(proc->page_table);

    kql_delete(proc->lock);
    kfree(proc);

    kql_release(proc_lock);
}

int kproc_wait(pid_t pid)
{
    process_t* proc = get_process(pid);
    kql_acquire(proc->lock);

    // wait for the process to finish
    while (proc->state != PROC_DEAD) {
        kevent_wait(proc->on_finish, proc->lock);
    }

    int code = proc->exit_code;
    delete_proc(proc);
    // no need to release proc->lock, we just deleted it
    return code;
}