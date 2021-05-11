#pragma once

#ifdef __is_libk
#include "sync/event.h"
#define EVENT_T         event_t*
#define EVENT_CREATE    kevent_create
#define EVENT_DELETE    kevent_delete
#define EVENT_WAIT      kevent_wait
#define EVENT_SIGNAL    kevent_signal
#define EVENT_BROADCAST kevent_broadcast
#endif

#ifdef __is_libc
#include <user_event.h>
#define EVENT_T         event_id_t
#define EVENT_CREATE    event_create
#define EVENT_DELETE    event_delete
#define EVENT_WAIT      event_wait
#define EVENT_SIGNAL    event_signal
#define EVENT_BROADCAST event_broadcast
#endif
