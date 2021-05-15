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

    char* cwd = malloc(32 * sizeof(char));
    getcwd(cwd, 32);

    printf("cwd=%s\n", cwd);
    chdir("/progs");

    getcwd(cwd, 32);

    printf("cwd=%s\n", cwd);

    printf("exit\n");
    proc_exit(42);

    while(1);
}







