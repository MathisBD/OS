#include <stdio.h>
#include <user_file.h>
#include <user_heap.h>



int main()
{
    printf("echo!!\n");
    /*for (int i = 0; i < argc; i++) {
        printf(argv[i]);
        if (i < argc-1) {
            printf(" ");
        }
    }*/

    

    extern void print_lists();
    print_lists();

    while(1);
    return 0;
}







