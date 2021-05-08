#include <blocking_queue.h>


void bq_delete(blocking_queue_t* q)
{
    kql_acquire(q->lock);

    kevent_delete(q->on_add);
    kevent_delete(q->on_remove);
    kfree(q->buffer);
    kql_delete(q->lock);
    kfree(q);
}