#include <stdio.h>
#include <user_file.h>




int main(int argc, char** argv)
{
    for (int i = 0; i < argc; i++) {
        printf(argv[i]);
        if (i < argc-1) {
            printf(" ");
        }
    }
    return 0;
}







