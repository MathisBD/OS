#include <stdio.h>
#include <user_file.h>
#include <user_heap.h>
#include <str.h>
#include <asm_debug.h>


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

    while(1);
    return 0;
}







