
#include <stdio.h>
#include <stdlib.h>
#include "memory/kheap.h"
#include <user_process.h>
#include <user_thread.h>
#include "sync/queuelock.h"
#include "sync/spinlock.h"
#include "threads/scheduler.h"


#define DELAY() {for (int a = 0; a < 100; a++);}

typedef struct {
    int* ptr;
    queuelock_t* lock;
} arg_t;

spinlock_t* ll;

void fn(arg_t* arg)
{
    for (int i = 0; i < 1000000; i++) {
        queuelock_acquire(arg->lock);
        int local = *(arg->ptr);
        DELAY();
        *(arg->ptr) = local + 1;
        queuelock_release(arg->lock);
    }
}



void kernel_main()
{
    ll = spinlock_create();

    arg_t* arg = kmalloc(sizeof(arg_t));
    arg->ptr = kmalloc(sizeof(int));
    *(arg->ptr) = 0;
    arg->lock = queuelock_create();


    tid_t tids[2];
    for (int i = 0; i < 2; i++) {
        tids[i] = thread_create(fn, arg);
        printf("created i = %d\n", i);
    }


    for (int i = 0; i < 2; i++) {
        thread_join(tids[i]);
    }

    printf("n=%d\n", *(arg->ptr));

    /*pid_t pid = proc_fork();
    printf("pid=%u\n", pid);

    if (pid == 0) {
        proc_exec("/user/user.o");
    }*/

    while (1);
}
