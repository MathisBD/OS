#pragma once

#if defined(__is_libk) || defined(__is_kernel)
#include "sync/queuelock.h"
#include "sync/event.h"
#include <stdint.h>

// blocking FIFO queue : when a thread tries to add to a full queue
// of remove from an empty queue, it waits (blocks) until some other thread
// changes the state of the queue.
typedef struct {
    // we use buffer as a circular buffer.
    void* buffer;
    uint32_t start; // offset of the first byte in the buffer
    uint32_t count; // current number of bytes
    uint32_t capacity; // maximum number of bytes

    queuelock_t* lock;
    event_t* on_add;
    event_t* on_remove;
} blocking_queue_t;


blocking_queue_t* bq_create(uint32_t capacity);
void bq_delete(blocking_queue_t* bq);
void bq_add(blocking_queue_t* queue, void* buf, uint32_t count);
void bq_remove(blocking_queue_t* queue, void* buf, uint32_t count);

#endif
