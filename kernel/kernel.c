
#include <stdio.h>
#include <stdlib.h>
#include "memory/kheap.h"
#include <user_process.h>
#include <user_thread.h>
#include <user_lock.h>

#define DELAY() {for (int i = 0; i < 0; i++);}

typedef struct {
    int* ptr;
    lock_id_t lock;
} arg_t;

void fn(arg_t* arg)
{
    for (int i = 0; i < 1000000; i++) {
        lock_acquire(arg->lock);
        int local = *(arg->ptr);
        DELAY();
        *(arg->ptr) = local + 1;
        lock_release(arg->lock);
    }
}


void kernel_main()
{
    arg_t* arg = kmalloc(sizeof(arg_t));
    arg->ptr = kmalloc(sizeof(int));
    *(arg->ptr) = 0;
    arg->lock = lock_create();

    tid_t tids[2];
    for (int i = 0; i < 2; i++) {
        tids[i] = thread_create(fn, arg);
        printf("created i = %d\n", i);
    }
    for (int i = 0; i < 2; i++) {
        int code = thread_join(tids[i]);
        printf("code for %d = %d\n", i, code);
    }

    printf("n=%d\n", *(arg->ptr));

    /*pid_t pid = proc_fork();
    printf("pid=%u\n", pid);

    if (pid == 0) {
        proc_exec("/user/user.o");
    }*/

    while (1);
}
