#include "interrupts/syscall.h"
#include "threads/process.h"
#include "threads/thread.h"
#include "threads/scheduler.h"
#include "filesystem/file_descr.h"
#include "sync/event.h"
#include "sync/queuelock.h"
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
        kproc_exec(get_syscall_arg(frame, 1));
        return;
    }
    case SC_PROC_EXIT:
    {
        kproc_exit(get_syscall_arg(frame, 1));
        return;
    }
    case SC_PROC_WAIT:
    {
        frame->eax = kproc_wait(get_syscall_arg(frame, 1));
        return;
    }
    case SC_THREAD_CREATE:
    {
        frame->eax = kthread_create(
            get_syscall_arg(frame, 1), 
            get_syscall_arg(frame, 2));
        return;
    }
    case SC_THREAD_YIELD:
    {
        kthread_yield();
        return;
    }
    case SC_THREAD_JOIN:
    {
        frame->eax = kthread_join(get_syscall_arg(frame, 1));
        return;
    }
    case SC_THREAD_EXIT:
    {
        kthread_exit(get_syscall_arg(frame, 1));
        return;
    }
    case SC_QL_CREATE:
    {
        queuelock_t* lock = kql_create();
        frame->eax = proc_add_lock(curr_process(), lock);
        return;
    }
    case SC_QL_DELETE:
    {
        queuelock_t* lock = proc_remove_lock(
            curr_process(), 
            get_syscall_arg(frame, 1));
        kql_delete(lock);
        return;
    }
    case SC_QL_ACQUIRE:
    {
        queuelock_t* lock = proc_get_lock(
            curr_process(),
            get_syscall_arg(frame, 1));
        kql_acquire(lock);
        return;
    }
    case SC_QL_RELEASE:
    {
        queuelock_t* lock = proc_get_lock(
            curr_process(),
            get_syscall_arg(frame, 1));
        kql_release(lock);
        return;
    }
    case SC_OPEN:
    {
        file_descr_t* fd = kopen(
            get_syscall_arg(frame, 1),
            get_syscall_arg(frame, 2));
        frame->eax = proc_add_fd(curr_process(), fd);
        return;
    }
    case SC_CLOSE:
    {
        file_descr_t* fd = proc_remove_fd(
            curr_process(),
            get_syscall_arg(frame, 1));
        kclose(fd);
        return;
    }
    case SC_READ:
    {
        file_descr_t* fd = proc_get_fd(
            curr_process(),
            get_syscall_arg(frame, 1));
        frame->eax = kread(
            fd,
            get_syscall_arg(frame, 2),
            get_syscall_arg(frame, 3));
        return;
    }
    case SC_WRITE:
    {
        file_descr_t* fd = proc_get_fd(
            curr_process(),
            get_syscall_arg(frame, 1));
        frame->eax = kwrite(
            fd,
            get_syscall_arg(frame, 2),
            get_syscall_arg(frame, 3));
        return;
    }
    /*case SC_PIPE:
    {
        kpipe();
        return;
    }
    case SC_DUP:
    {
        kdup();
        return;
    }*/
    case SC_EVENT_CREATE:
    {
        queuelock_t* lock = proc_get_lock(
            curr_process(),
            get_syscall_arg(frame, 1));
        event_t* event = kevent_create(lock);
        frame->eax = proc_add_event(curr_process(), event);
        return;
    }
    case SC_EVENT_DELETE:
    {
        event_t* event = proc_remove_event(
            curr_process(), 
            get_syscall_arg(frame, 1));
        kevent_delete(event);
        return;
    }
    case SC_EVENT_WAIT:
    {
        event_t* event = proc_get_event(
            curr_process(), 
            get_syscall_arg(frame, 1));
        kevent_wait(event);
        return;
    }
    case SC_EVENT_SIGNAL:
    {
        event_t* event = proc_get_event(
            curr_process(), 
            get_syscall_arg(frame, 1));
        kevent_signal(event);
        return;
    }
    case SC_EVENT_BROADCAST:
    {
        event_t* event = proc_get_event(
            curr_process(), 
            get_syscall_arg(frame, 1));
        kevent_broadcast(event);
        return;
    }
    default:
        printf("Unknown system call !\nsyscall number=0x%x\n", frame->eax);
        while(1);
    }
}
