#ifdef __is_libk
#include "threads/lock.h"

lock_id_t lock_create()
{
    return do_lock_create();
}

#endif 