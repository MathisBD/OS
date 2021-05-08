#ifdef __is_libk
#include <blocking_queue.h>
#include <string.h>


static uint32_t min(uint32_t a, uint32_t b)
{
    return a < b ? a : b;
}

// assumes we hold the lock on the queue.
// assumes count <= q->count.
static void do_remove(blocking_queue_t* q, void* buf, uint32_t count)
{
    uint32_t space = q->capacity - q->start;
    if (count <= space) {
        memcpy(buf, q->buffer + q->start, count);
    }
    else {
        memcpy(buf, q->buffer + q->start, space);
        memcpy(buf + space, q->buffer, count - space);
    }
    q->start = (q->start + count) % q->capacity;
    q->count -= count;
}


void bq_remove(blocking_queue_t* q, void* buf, uint32_t count)
{
    queuelock_acquire(q->lock);

    uint32_t ofs = 0;
    while (ofs < count) {
        while (q->count == 0) {
            event_wait(q->on_add);
        }
        // how much bytes can we remove ?
        uint32_t amount = min(count - ofs, q->count);
        do_remove(q, buf + ofs, amount);
        ofs += amount;
        // tell others we removed from the queue.
        event_broadcast(q->on_remove);
    }
    queuelock_release(q->lock);
}
#endif