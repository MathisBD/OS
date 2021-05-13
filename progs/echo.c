#include <stdio.h>
#include <user_heap.h>
#include <user_process.h>
#include <str.h>


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

    proc_exit(42);

    while(1);
    return 0;
}







