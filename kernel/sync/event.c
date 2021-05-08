#include "sync/event.h"
#include "threads/scheduler.h"
#include "memory/kheap.h"
#include <panic.h>

static void check_monitor(event_t* event)
{
    if (!queuelock_is_held(event->monitor)) {
        panic("can't interact with an event when we don't hold it's monitor lock");
    }
}

event_t* kevent_create(queuelock_t* lock)
{
    event_t* event = kmalloc(sizeof(event_t));
    event->waiting = list_create();
    event->monitor = lock;
    return event;
}

void kevent_delete(event_t* event)
{
    check_monitor(event);
    if (!list_empty(event->waiting)) {
        panic("can't delete an event when threads are waiting on it\n");
    }
    // don't delete the monitor, we don't own it.
    list_delete(event->waiting);
    kfree(event);
}

void kevent_wait(event_t* event)
{
    check_monitor(event);

    list_add_back(event->waiting, curr_thread());
    sched_suspend_and_release_queuelock(event->monitor);
    queuelock_acquire(event->monitor);
}

void kevent_signal(event_t* event)
{
    check_monitor(event);

    if (!list_empty(event->waiting)) {
        thread_t* next = list_pop_front(event->waiting);
        sched_wake_up(next);
    }
}

void kevent_broadcast(event_t* event)
{
    check_monitor(event);

    while (!list_empty(event->waiting)) {
        thread_t* next = list_pop_front(event->waiting);
        sched_wake_up(next);
    }
}
