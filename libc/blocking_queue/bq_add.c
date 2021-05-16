#include <stdint.h>
#include <blocking_queue.h>
#include <string.h>
#include "lock_macros.h"
#include "event_macros.h"

static uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

// assumes we hold the lock on the queue.
// assumes q->count + count <= q->capacity.
static void do_add(blocking_queue_t* q, void* buf, uint32_t count)
{
    uint32_t end = (q->start + q->count) % q->capacity;
    uint32_t space = q->capacity - end;
    if (count <= space) {
        memcpy(q->buffer + end, buf, count);
    }
    else {
        memcpy(q->buffer + end, buf, space);
        memcpy(q->buffer, buf + space, count - space);
    }
    // q->start doesn't change
    q->count += count;
}


void bq_add(blocking_queue_t* q, void* buf, uint32_t count)
{
    LOCK_ACQUIRE(q->lock);

    uint32_t ofs = 0;
    while (ofs < count) {
        while (q->count >= q->capacity) {
            EVENT_WAIT(q->on_remove, q->lock);
        }
        // how much bytes can we add ?
        uint32_t amount = min(count - ofs, q->capacity - q->count);
        do_add(q, buf + ofs, amount);
        ofs += amount;
        // tell others we added to the queue.
        EVENT_BROADCAST(q->on_add);
    }
    LOCK_RELEASE(q->lock);
}