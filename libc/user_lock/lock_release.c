#ifdef __is_libk
#include "threads/lock.h"

void lock_release(lock_id_t lock)
{
    do_lock_release(lock);
}

#endif