#include "drivers/dev.h"
#include <list.h>

static list_t* block_devs;
static list_t* stream_devs;


void init_dev()
{
    block_devs = list_create();
    stream_devs = list_create();
}


stream_dev_t* register_stream_dev(char* name, uint8_t flags)
{
    stream_dev_t* dev = kmalloc(sizeof(stream_dev_t));
    dev->lock = kql_create();
    dev->name = name;
    dev->flags = flags;
    if (flags & DEV_FLAG_READ) {
        dev->read_buf = bq_create(STREAM_DEV_BUF_SIZE);
    }
    else {
        dev->read_buf = 0;
    }
    if (flags & DEV_FLAG_WRITE) {
        dev->write_buf = bq_create(STREAM_DEV_BUF_SIZE);
    }
    else {
        dev->write_buf = 0;
    }

    list_add_front(stream_devs, dev);
    return dev;
}

block_dev_t* register_block_dev(char* name, uint8_t flags)
{
    return 0;
}

stream_dev_t* get_stream_dev(char* name)
{
    for (list_node_t* node = block_devs->first; node != 0; node = node->next) {
        stream_dev_t* dev = node->contents;
        if (dev->name == name) {
            return dev;
        }
    }
    return 0;
}

block_dev_t* get_block_dev(char* name)
{
    return 0;
}

void unregister_stream_dev(stream_dev_t* dev)
{
    for (list_node_t* node = block_devs->first; node != 0; node = node->next) {
        if (node->contents == dev) {
            list_remove_node(stream_devs, node);
        }
    }
    panic("didn't find streaming device to unregister\n");
}

void unregister_block_dev(block_dev_t* dev);
