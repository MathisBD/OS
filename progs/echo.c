#include <stdio.h>
#include <user_heap.h>
#include <user_process.h>
#include <user_thread.h>
#include <user_file.h>
#include <str.h>

void fn(int arg)
{
    printf("reading key\n");
    char c;
    read(FD_STDIN, &c, 1);
    printf("done\n");
}

int _main(int argc, char** argv)
{
    init_heap();
    //printf("echo!!\n");

    //printf("argc=%d, argv=%x\n", argc, argv);

    for (int i = 0; i < argc; i++) {
        printf(argv[i]);
        if (i < argc-1) {
            printf(" ");
        }
    }

    thread_create(fn, 0);
    for (int i = 0; i < 100000000; i++);

    printf("exit\n");
    proc_exit(42);

    while(1);
    return 0;
}







