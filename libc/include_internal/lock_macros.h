#pragma once

#ifdef __is_libk
#include "sync/queuelock.h"
#define LOCK_T          queuelock_t*
#define LOCK_CREATE     kql_create
#define LOCK_DELETE     kql_delete
#define LOCK_ACQUIRE    kql_acquire
#define LOCK_RELEASE    kql_release
#endif 


#ifdef __is_libc
#include <user_lock.h>
#define LOCK_T          lock_id_t
#define LOCK_CREATE     lock_create
#define LOCK_DELETE     lock_delete
#define LOCK_ACQUIRE    lock_acquire
#define LOCK_RELEASE    lock_release
#endif 


