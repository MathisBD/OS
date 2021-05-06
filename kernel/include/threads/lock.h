#pragma once
#include <stdint.h>
#include <list.h>
#include "threads/_types.h"


lock_id_t do_lock_create();
void do_lock_delete(lock_id_t);
void do_lock_acquire(lock_id_t);
void do_lock_release(lock_id_t);
