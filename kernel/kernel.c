
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


#define DELAY() {for (int a = 0; a < 100; a++);}


void kernel_main()
{
    ////// TODO : replace the heap spinlock with a queuelock
    // (by changing the queuelock implementation to not use the heap anymore,
    // and use instead the sched_next field of thread_t);
    printf("kernel\n");

    /*uint16_t key;
    while (true) {
        read(FD_STDIN, &key, 2);
        printf("%c", (char)(key & 0xFF));
    }*/

    file_descr_t* fd = kopen("/dev/vga_buffer", FD_PERM_WRITE | FD_PERM_SEEK | FD_PERM_READ);
    kseek(fd, 6, FD_SEEK_SET);
    uint16_t c = 'H';
    c |= 0x0A00;
    kwrite(fd, &c, 2);

    uint8_t* buf = kmalloc(16);
    kseek(fd, 0, FD_SEEK_SET);
    kread(fd, buf, 16);
    for (int i = 0; i < 16; i++) {
        printf("%02x ", buf[i]);
    }
    
    /*for (int i = 0; i < 8; i++) {
        thread_create(allocate, 0);
    }

    printf("created threads\n");*/


    /*arg_t* arg = kmalloc(sizeof(arg));
    arg->q = bq_create(18);

    tid_t tids[2];
    tids[0] = thread_create(writer, arg);
    tids[1] = thread_create(reader, arg);

    for (int i = 0; i < 2; i++) {
        thread_join(tids[i]);
    }

    printf("done\n");


    pid_t pid = proc_fork();
    printf("pid=%u\n", pid);

    if (pid != 0) {*/
    /*printf("hello\n");
    arg_t* arg = kmalloc(sizeof(arg_t));
    arg->ptr = kmalloc(sizeof(int));
    *(arg->ptr) = 0;
    arg->lock = lock_create();

    tid_t tids[8];
    for (int i = 0; i < 8; i++) {
        tids[i] = thread_create(fn, arg);
        printf("created i = %d\n", i);
    }


    for (int i = 0; i < 8; i++) {
        thread_join(tids[i]);
    }

    printf("n=%d\n", *(arg->ptr));*/
    //}

    /*if (pid == 0) {
        proc_exec("/user/user.o");
    }*/

    while (1);
}
