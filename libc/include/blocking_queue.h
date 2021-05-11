#pragma once
#include <stdint.h>

#if defined(__is_libk) || defined(__is_kernel)
#include "sync/event.h"
#include "sync/queuelock.h"
#define LOCK_T      queuelock_t*
#define EVENT_T     event_t*
#else 
#include <user_event.h>
#include <user_lock.h>
#define LOCK_T      lock_id_t
#define EVENT_T     event_id_t
#endif

// blocking FIFO queue : when a thread tries to add to a full queue
// of remove from an empty queue, it waits (blocks) until some other thread
// changes the state of the queue.
typedef struct {
    // we use buffer as a circular buffer.
    void* buffer;
    uint32_t start; // offset of the first byte in the buffer
    uint32_t count; // current number of bytes
    uint32_t capacity; // maximum number of bytes

    LOCK_T lock;
    EVENT_T on_add;
    EVENT_T on_remove;
} blocking_queue_t;


blocking_queue_t* bq_create(uint32_t capacity);
void bq_delete(blocking_queue_t* bq);
void bq_add(blocking_queue_t* queue, void* buf, uint32_t count);
void bq_remove(blocking_queue_t* queue, void* buf, uint32_t count);

