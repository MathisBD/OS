#include <blocking_queue.h>
#include "memory/kheap.h"
#include <panic.h>

blocking_queue_t* bq_create(uint32_t capacity)
{
    if (capacity == 0) {
        panic("can't create a blocking queue with capacity=0");
    }

    blocking_queue_t* q = kmalloc(sizeof(blocking_queue_t));
    q->buffer = kmalloc(capacity);
    q->start = 0;
    q->count = 0;
    q->capacity = capacity;
    q->lock = queuelock_create();
    q->on_add = event_create(q->lock);
    q->on_remove = event_create(q->lock);
    return q;
}