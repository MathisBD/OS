
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


#define DELAY() {for (int a = 0; a < 100000000; a++);}

void fn(int arg)
{
    printf("arg=%d\n", arg);
    DELAY();
    while(1);
}


void kernel_main()
{
    pid_t pid = proc_fork();
    printf("pid=%u\n", pid);

    /*tid_t tid = thread_create(fn, 42);
    //thread_join(tid);
    printf("after\n");*/


    // child
    if (pid == 0) {
        printf("child\n");
        char** argv = kmalloc(0*sizeof(char*));
        kproc_exec("/progs/shell.elf", 0, 0);
    }
    // parent
    int c = proc_wait(pid);
    printf("child finished with %d\n", c);
    
    while (1);
}
