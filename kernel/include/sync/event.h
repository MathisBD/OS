#pragma once
#include <list.h>
#include "sync/queuelock.h"


// an event is always "monitored" by a lock.
// event functions can only be called if we hold the monitor lock.
// this is why we don't need an internal lock in the struct.
typedef struct {
    // list of threads waiting on this event
    list_t* waiting;
    queuelock_t* monitor;
} event_t;

event_t* event_create(queuelock_t* lock);
void event_delete(event_t* event);

// event_wait() atomically releases the monitor lock and
// suspends the current thread ; when the thread is woken up it acquires
// the lock again before returning.
// event_wait() should always be called inside a loop :
// while(!condition) {
//     event_wait(event);    
//}
void event_wait(event_t* event);
void event_signal(event_t* event);
void event_broadcast(event_t* event);
