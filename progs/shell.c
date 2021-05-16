#include <user_file.h>
#include <stdio.h>
#include <user_process.h>
#include <user_heap.h>
#include <str.h>
#include <vect.h>
#include <list.h>


#define CWD_SIZE    64
char* cwd;

void print_header()
{
    getcwd(cwd, CWD_SIZE);
    printf("%s >>", cwd);
}

// read space separated words,
// printing them as they are read.
// terminates when a newline is read.
vect_t* read_words()
{
    vect_t* words = vect_create();
 
    char c;
    str_t* w = str_create();
    while (1) {
        read(FD_STDIN, &c, 1);
        switch(c) {
        case 0:
            // don't do anything
            break;
        case ' ':
            putchar(c);
            vect_append(words, w);
            w = str_create();
            break;
        case '\n':
            putchar(c);
            vect_append(words, w);
            return words;
        default:
            putchar(c);
            str_add_char(w, c);
            break;
        }
    }
    return 0;
}

void _main(int argc, char** argv)
{
    init_heap();
    printf("shell!!\n");
    cwd = malloc(CWD_SIZE);

    while (1) {
        print_header();

        printf("A\n");
        void* buf = malloc(10);
        printf("B\n");

        vect_t* words = read_words();
        for (int i = 0; i < words->size; i++) {
            str_t* w = vect_get(words, i);
            printf("got word : %s\n", str_get_cstr(w));
        }

        if (words->size > 0) {
            char* prog = str_get_cstr(vect_get(words, 0));
            int argc = words->size - 1;
            char** argv = malloc(argc);
            for (int i = 0; i < argc; i++) {
                str_t* arg = vect_get(words, i+1);
                argv[i] = str_get_cstr(arg);
            }
            printf("FORK\n");
            pid_t pid = proc_fork();
            //while(1);
            
            // child
            if (pid == 0) {
                printf("EXEC\n");
                proc_exec(prog, argc, argv);
            }
            // parent 
            else {
                proc_wait(pid);
            }
        }
    }
   
    /*while (read_words() {
        pid_t pid = proc_fork();

        // child
        if (pid == 0) {
            proc_exec(prog, argc, argv);
        }
        // still the shell
        else {
            proc_wait(pid);
        }
    }*/
    while(1);
}