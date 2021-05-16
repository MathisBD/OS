#include <blocking_queue.h>
#include "heap_macros.h"
#include "event_macros.h"
#include "lock_macros.h"
#include <panic.h>
#include <stdio.h>

blocking_queue_t* bq_create(uint32_t capacity)
{
    if (capacity == 0) {
        panic("can't create a blocking queue with capacity=0");
    }

    blocking_queue_t* q = MALLOC(sizeof(blocking_queue_t));
    q->buffer = MALLOC(capacity);
    q->start = 0;
    q->count = 0;
    q->capacity = capacity;
    q->lock = LOCK_CREATE();
    q->on_add = EVENT_CREATE();
    q->on_remove = EVENT_CREATE();
    return q;
}