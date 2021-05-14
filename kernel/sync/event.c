#include "sync/event.h"
#include "threads/scheduler.h"
#include "memory/kheap.h"
#include <panic.h>


// assumes we hold the monitor lock of the event
static void add_waiting(event_t* e, thread_t* t)
{
    if (e->wait_first == 0) {
        e->wait_first = e->wait_last = t;
    }
    else {
        e->wait_last->wait_next = t;
        e->wait_last = t;
    }
}

static void has_waiting(event_t* e)
{
    return e->wait_first != 0;
}

static thread_t* pop_waiting(event_t* e)
{
    if (e->wait_first == 0) {
        return 0;
    }
    else if (e->wait_first == e->wait_last) {
        thread_t* t = e->wait_first;
        e->wait_first = e->wait_last = 0;
        return t;
    }
    else {
        thread_t* t = e->wait_first;
        e->wait_first = t->wait_next;
        return t;
    }
}

static void check_monitor(event_t* event)
{
    if (!kql_is_held(event->monitor)) {
        panic("can't interact with an event when we don't hold it's monitor lock");
    }
}

event_t* kevent_create(queuelock_t* lock)
{
    event_t* event = kmalloc(sizeof(event_t));
    event->wait_first = event->wait_last = 0;
    event->monitor = lock;
    return event;
}

void kevent_delete(event_t* event)
{
    check_monitor(event);
    if (event->wait_first != 0) {
        panic("can't delete an event when threads are waiting on it\n");
    }
    // don't delete the monitor, we don't own it.
    kfree(event);
}

void kevent_wait(event_t* event)
{
    check_monitor(event);

    add_waiting(event, curr_thread());
    sched_suspend_and_release_queuelock(event->monitor);
    kql_acquire(event->monitor);
}

void kevent_signal(event_t* event)
{
    check_monitor(event);

    if (has_waiting(event->wait_first)) {
        thread_t* next = pop_waiting(event);
        sched_wake_up(next);
    }
}

void kevent_broadcast(event_t* event)
{
    check_monitor(event);

    while (has_waiting(event)) {
        thread_t* next = pop_waiting(event->waiting);
        sched_wake_up(next);
    }
}
