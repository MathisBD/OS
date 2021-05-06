#ifdef __is_libk
#include "threads/thread.h"

tid_t thread_create(void(*func)(int), int arg)
{
    return do_thread_create(func, arg);
}

#endif