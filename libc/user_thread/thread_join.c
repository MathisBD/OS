#ifdef __is_libk
#include "threads/thread.h"

void thread_join(tid_t tid)
{
    do_thread_join(tid);
}

#endif