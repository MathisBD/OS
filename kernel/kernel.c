
#include <stdio.h>
#include <stdlib.h>
#include "memory/kheap.h"
#include <user_process.h>
#include <user_thread.h>
#include "sync/queuelock.h"
#include "sync/spinlock.h"
#include "threads/scheduler.h"
#include <blocking_queue.h>


#define DELAY() {for (int a = 0; a < 10000; a++);}

typedef struct {
    blocking_queue_t* q;
} arg_t;


void writer(arg_t* arg)
{
    uint32_t N = 5;
    char* buf = kmalloc(N);

    for (int i = 0; i < 10; i++) {
        for (int c = 0; c < N; c++) {
            buf[c] = c + i;
        }
        bq_add(arg->q, buf, N);
        DELAY();
    }
}

void reader(arg_t* arg)
{
    uint32_t N = 5;
    char* buf = kmalloc(N);

    for (int i = 0; i < 10; i++) {
        bq_remove(arg->q, buf, N);
        print_mem(buf, N);
        printf("\n");

        DELAY();
    }
}


void kernel_main()
{
    arg_t* arg = kmalloc(sizeof(arg));
    arg->q = bq_create(18);

    tid_t tids[2];
    tids[0] = thread_create(writer, arg);
    tids[1] = thread_create(reader, arg);

    for (int i = 0; i < 2; i++) {
        thread_join(tids[i]);
    }

    printf("done\n");


    /*pid_t pid = proc_fork();
    printf("pid=%u\n", pid);

    if (pid != 0) {
        printf("hello\n");
        arg_t* arg = kmalloc(sizeof(arg_t));
        arg->ptr = kmalloc(sizeof(int));
        *(arg->ptr) = 0;
        arg->lock = queuelock_create();
        tid_t tids[8];
        for (int i = 0; i < 8; i++) {
            tids[i] = thread_create(fn, arg);
            printf("created i = %d\n", i);
        }


        for (int i = 0; i < 8; i++) {
            thread_join(tids[i]);
        }

        printf("n=%d\n", *(arg->ptr));
    }

    if (pid == 0) {
        proc_exec("/user/user.o");
    }*/

    while (1);
}
