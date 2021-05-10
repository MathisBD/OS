#include "drivers/dev.h"
#include <list.h>
#include <panic.h>
#include "sync/queuelock.h"
#include <stdio.h>
#include <string.h>


static queuelock_t* dev_lock;
static list_t* block_devs;
static list_t* stream_devs;


void init_dev()
{
    dev_lock = kql_create();
    block_devs = list_create();
    stream_devs = list_create();
}


void register_stream_dev(stream_dev_t* dev)
{
    kql_acquire(dev_lock);
    list_add_front(stream_devs, dev);
    kql_release(dev_lock);
    printf("registered device %s\n", dev->name);
}

void register_block_dev(block_dev_t* dev)
{
    return 0;
}

stream_dev_t* get_stream_dev(char* name)
{
    kql_acquire(dev_lock);
    for (list_node_t* node = stream_devs->first; node != 0; node = node->next) {
        stream_dev_t* dev = node->contents;
        if (memcmp(dev->name, name, strlen(name)) == 0) {
            return dev;
        }
    }
    return 0;
    kql_release(dev_lock);
}

block_dev_t* get_block_dev(char* name)
{
    return 0;
}

void unregister_stream_dev(stream_dev_t* dev)
{
    kql_acquire(dev_lock);
    for (list_node_t* node = block_devs->first; node != 0; node = node->next) {
        if (node->contents == dev) {
            list_remove_node(stream_devs, node);
            kql_release(dev_lock);
            return;
        }
    }
    panic("didn't find streaming device to unregister\n");
    kql_release(dev_lock);
}

void unregister_block_dev(block_dev_t* dev)
{

}
