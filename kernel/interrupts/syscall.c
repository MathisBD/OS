#include "interrupts/syscall.h"
#include "threads/process.h"
#include "threads/thread.h"
#include "threads/scheduler.h"
#include "filesystem/file_descr.h"
#include "sync/event.h"
#include "sync/queuelock.h"
#include <panic.h>
#include <stdio.h>
#include <string.h>
#include "drivers/vga_driver.h"
#include <asm_debug.h>


uint32_t get_syscall_arg(intr_frame_t* frame, uint32_t arg)
{
    switch(arg) {
    case 0: return frame->eax;
    case 1: return frame->ebx;
    case 2: return frame->ecx;
    case 3: return frame->edx;
    case 4: return frame->edi;
    case 5: return frame->esi;
    default: panic("invalid syscall arg index (get)\n"); return 0;
    }
}

void set_syscall_arg(intr_frame_t* frame, uint32_t arg, uint32_t value)
{
    switch(arg) {
    case 0: frame->eax = value; return;
    case 1: frame->ebx = value; return;
    case 2: frame->ecx = value; return;
    case 3: frame->edx = value; return;
    case 4: frame->edi = value; return;
    case 5: frame->esi = value; return;
    default: panic("invalid syscall arg index (set)\n"); return;
    }
}


void handle_syscall(intr_frame_t* frame) 
{
	/*uint32_t esp = get_esp();
	vga_print("syscall esp=");
	vga_print_mem(&esp, 4);
	vga_print("\n");*/

    switch (frame->eax) {
    case SC_PROC_FORK:
    {
		do_proc_fork(frame);
		return;
    }
    case SC_PROC_EXEC:
    {
        kproc_exec(
            get_syscall_arg(frame, 1),
            get_syscall_arg(frame, 2),
            get_syscall_arg(frame, 3));
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
    case SC_PIPE:
    {
        uint32_t* p_from_id = get_syscall_arg(frame, 1);
        uint32_t* p_to_id = get_syscall_arg(frame, 2);

        file_descr_t* from;
        file_descr_t* to;
        kpipe(&from, &to);
        *p_from_id = proc_add_fd(curr_process(), from);
        *p_to_id = proc_add_fd(curr_process(), to);
        return;
    }
    case SC_DUP:
    {
        file_descr_t* original = proc_get_fd(
            curr_process(),
            get_syscall_arg(curr_process(), 1));
        if (original == 0) {
            panic("SC_DUP\n");
        }
        frame->eax = proc_add_fd(
            curr_process(), 
            fd_copy(original));
        return;
    }
    case SC_DUP2:
    {
        uint32_t original_id = get_syscall_arg(frame, 1);
        uint32_t copy_id = get_syscall_arg(frame, 2);
        if (original_id == copy_id) {
            return;
        }

        process_t* proc = curr_process();
        kql_acquire(proc->lock);

        if (original_id >= proc->file_descrs->size) {
            panic("SC_DUP2\n");
        }
        file_descr_t* original = vect_get(proc->file_descrs, original_id);
        if (original == 0) {
            panic("SC_DUP2\n");
        }
        file_descr_t* copy = fd_copy(original);
        if (copy_id >= proc->file_descrs->size) {
            panic("SC_DUP2\n");
        }
        file_descr_t* old = vect_set(proc->file_descrs, copy_id, copy);
        if (old != 0) {
            kclose(old);
        }

        kql_release(proc->lock);
        return;
    }
    case SC_SEEK:
    {
        file_descr_t* fd = proc_get_fd(
            curr_process(),
            get_syscall_arg(frame, 1));
        kseek(fd, 
            get_syscall_arg(frame, 2),
            get_syscall_arg(frame, 3));
        return;   
    }
    case SC_EVENT_CREATE:
    {
        event_t* event = kevent_create();
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
        queuelock_t* lock = proc_get_lock(
            curr_process(),
            get_syscall_arg(frame, 2));
        kevent_wait(event, lock);
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
    case SC_PROC_DATA_SIZE:
    {
        process_t* proc = curr_process();
        kql_acquire(proc->lock);
        uint32_t size = proc->data_size;
        kql_release(proc->lock);
        frame->eax = size;
        return;
    }
    case SC_PROC_STACK_SIZE:
    {
        process_t* proc = curr_process();
        kql_acquire(proc->lock);
        uint32_t size = proc->stack_size;
        kql_release(proc->lock);
        frame->eax = size;
        return;
    }
    case SC_CREATE:
    {
        kcreate(
            get_syscall_arg(frame, 1),
            get_syscall_arg(frame, 2));
        return;
    }
    case SC_REMOVE:
    {
        kremove(
            get_syscall_arg(frame, 1),
            get_syscall_arg(frame, 2));
        return;
    }
    case SC_GET_SIZE:
    {
        file_descr_t* fd = proc_get_fd(
            curr_process(), 
            get_syscall_arg(frame, 1));
        frame->eax = kget_size(fd);
        return;
    }
    case SC_RESIZE:
    {
        file_descr_t* fd = proc_get_fd(
            curr_process(), 
            get_syscall_arg(frame, 1));
        kresize(fd, get_syscall_arg(frame, 2));
        return;
    }
    case SC_LIST_DIR:
    {
        file_descr_t* fd = proc_get_fd(
            curr_process(), 
            get_syscall_arg(frame, 1));
        frame->eax = klist_dir(
            fd,
            get_syscall_arg(frame, 2),
            get_syscall_arg(frame, 3));
        return;
    }
    case SC_GETCWD:
    {
        process_t* proc = curr_process();
        kql_acquire(proc->lock);
        char* path = proc->cwd;
        kql_release(proc->lock);

        void* buf = get_syscall_arg(frame, 1);
        uint32_t size = get_syscall_arg(frame, 2);
        if (strlen(path) + 1 <= size) {
            memcpy(buf, path, strlen(path) + 1);
            frame->eax = 0;
        }
        else {
            memcpy(buf, path, size-1);
            memset(buf+size-1, 0, 1);
            frame->eax = 1;
        } 
        return;
    }
    case SC_CHDIR:
    {
        char* path = get_syscall_arg(frame, 1);
        if (kfile_type(path) != FD_TYPE_DIR) {
            panic("SC_CHDIR");
        }    
        process_t* proc = curr_process();
        kql_acquire(proc->lock);
        proc->cwd = path;
        kql_release(proc->lock);
        return;
    }
    case SC_FILE_TYPE:
    {
        char* path = get_syscall_arg(frame, 1);
        frame->eax = kfile_type(path);
        return;
    }
    default:
        printf("Unknown system call !\nsyscall number=0x%x\n", frame->eax);
        printf("registers : eax=%x, ebx=%x, ecx=%x, edx=%x, esi=%x, edi=%x\n",
			frame->eax, frame->ebx, frame->ecx, frame->edx, frame->esi, frame->edi);
        while(1);
    }
}
