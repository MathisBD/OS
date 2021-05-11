#include <blocking_queue.h>
#include "heap_macros.h"
#include "event_macros.h"
#include "lock_macros.h"


void bq_delete(blocking_queue_t* q)
{
    LOCK_ACQUIRE(q->lock);

    EVENT_DELETE(q->on_add);
    EVENT_DELETE(q->on_remove);
    FREE(q->buffer);
    LOCK_DELETE(q->lock);
    FREE(q);
}