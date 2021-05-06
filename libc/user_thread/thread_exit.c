#ifdef __is_libk
#include "threads/thread.h"

void thread_exit(int exit_code)
{
    do_thread_exit(exit_code);
}

#endif