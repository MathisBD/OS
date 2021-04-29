
#include <stdio.h>
#include "threads/thread.h"
#include "threads/scheduler.h"
#include <stdlib.h>


#define DELAY() {for (int i = 0; i < 100000000; i++);}


void fn(int arg)
{
    printf("thread : arg = %d\n", arg);
    DELAY();
    thread_exit(arg);
}


void kernel_main()
{
    tid_t tids[5];
    for (int i = 0; i < 5; i++) {
        tids[i] = thread_create(fn, i);
        printf("created i = %d\n", i);
    }
    for (int i = 0; i < 5; i++) {
        int code = thread_join(tids[i]);
        printf("code for %d = %d\n", i, code);
    }


    while (1);
}
