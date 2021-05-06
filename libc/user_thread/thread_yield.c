#ifdef __is_libk
#include "threads/thread.h"

void thread_yield()
{
    do_thread_yield();
}

#endif