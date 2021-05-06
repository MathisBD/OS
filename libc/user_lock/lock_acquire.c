#ifdef __is_libk
#include "threads/lock.h"

void lock_acquire(lock_id_t lock)
{
    do_lock_acquire(lock);
}

#endif