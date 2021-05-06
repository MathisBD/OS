#ifdef __is_libk
#include "threads/lock.h"

void lock_delete(lock_id_t lock)
{
    do_lock_delete(lock);
}

#endif
