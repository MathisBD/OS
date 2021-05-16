#pragma once
#include <stdint.h>

typedef uint32_t event_id_t;
typedef uint32_t lock_id_t;

event_id_t event_create();
void event_delete(event_id_t event_id);
void event_wait(event_id_t event_id, lock_id_t lock_id);
void event_signal(event_id_t event_id);
void event_broadcast(event_id_t event_id);