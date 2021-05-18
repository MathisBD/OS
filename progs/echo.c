#include <stdio.h>
#include <user_heap.h>
#include <user_process.h>
#include <user_thread.h>
#include <user_file.h>
#include <str.h>

void _main(int argc, char** argv)
{
    init_heap();

    for (int i = 0; i < argc; i++) {
        printf(argv[i]);
        if (i < argc-1) {
            printf(" ");
        }
    }
    printf("\n");
    proc_exit(0);
}







