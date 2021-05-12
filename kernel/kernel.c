
#include <stdio.h>
#include <stdlib.h>
#include "memory/kheap.h"
#include <user_process.h>
#include <user_thread.h>
#include "sync/queuelock.h"
#include "sync/spinlock.h"
#include "threads/scheduler.h"
#include <blocking_queue.h>
#include <user_thread.h>
#include <user_lock.h>
#include <string.h>
#include <user_file.h>
#include "drivers/vga_driver.h"


#define DELAY() {for (int a = 0; a < 500000000; a++);}

void fn(int arg)
{
    printf("arg=%d\n", arg);
    while(1);
}


void kernel_main()
{
    ////// TODO : replace the heap spinlock with a queuelock
    // (by changing the queuelock implementation to not use the heap anymore,
    // and use instead the sched_next field of thread_t);

    /*tid_t tid = kthread_create(fn, 42);
    //thread_join(tid);
    printf("after\n");*/

    pid_t pid = proc_fork();
    printf("pid=%u\n", pid);

    if (pid == 0) {
        kproc_exec("/progs/echo.elf", 0, 0);
    }
    DELAY();
    printf("execed!\n");

    while (1);
}
