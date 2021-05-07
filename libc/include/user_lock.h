#pragma once
#include <stdint.h>

#ifdef __is_libc
// a lock identifier. 
// each lock id is local to a process
typedef uint32_t lock_id_t;

lock_id_t lock_create();
void lock_delete(lock_id_t lock);
void lock_acquire(lock_id_t lock);
void lock_release(lock_id_t lock);
#endif 