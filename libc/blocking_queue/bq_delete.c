#include <blocking_queue.h>


void bq_delete(blocking_queue_t* q)
{
    queuelock_acquire(q->lock);

    event_delete(q->on_add);
    event_delete(q->on_remove);
    kfree(q->buffer);
    queuelock_delete(q->lock);
    kfree(q);
}