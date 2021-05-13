#include <user_file.h>
#include <stdio.h>
#include <user_process.h>
#include <user_heap.h>
#include <str.h>
#include <list.h>


int parse_command(char** p_prog, int* p_argc, char*** p_argv)
{
    str_t* prog;
    list_t* args; // list of str_t*

    char c;
    while (1) {
        read(FD_STDIN, &c, 1);
        putchar(c);
        if (c == ' ') {
            break;
        }
        else {
            str_add_char(prog, c);
        }
    }

    *p_prog = str_get_cstr(prog);
    *p_argc = 
}

int _main(int argc, char** argv)
{
    str_t prog;
    int argc;
    str_t* argv;
    while (parse_command(&prog, &argc, &argv)) {
        pid_t pid = proc_fork();

        // child
        if (pid == 0) {
            proc_exec(prog, argc, argv);
        }
        // still the shell
        else {
            proc_wait(pid);
        }
    }
    return 0;
}