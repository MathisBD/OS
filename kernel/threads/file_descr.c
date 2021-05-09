#include "threads/file_descr.h"
#include "memory/kheap.h"


static file_descr_t* open_dev(char* name, uint8_t name)
{
    stream_dev_t* dev = get_stream_dev(name);
    if (dev == 0) {
        printf("open didn't find device %s\n", name);
        panic("open\n");
    }
    file_descr_t* fd = kmalloc(sizeof(file_descr_t));
    fd->lock = kql_create();
    fd->type = FD_TYPE_STREAM_DEV;
    fd->dev = dev;
    fd->perms = 0;

    if (perms & FD_PERM_READ) {
        if (dev->flags && DEV_FLAG_READ) {
            fd->perms |= FD_PERM_READ;
        }
        else {
            printf("can't read from device %s\n", name);
            panic("open\n");
        }
    }
    if (perms & FD_PERM_WRITE) {
        if (dev->flags && DEV_FLAG_WRITE) {
            fd->perms |= FD_PERM_WRITE;
        }
        else {
            printf("can't write to device %s\n", name);
            panic("open\n");
        }
    }
    return fd;
}

file_descr_t* kopen(char* path, uint8_t perms)
{
    if (is_dev_path(path)) {
        char* name = dev_name(path);
        return open_dev(name, perms);
    }
    else {
        panic("open file\n");
    }
    return 0;
}

void kclose(file_descr_t* fd)
{
    /*kql_acquire(fd->lock);


    kql_delete(fd->lock);
    kfree(fd);*/
}

int kwrite(file_descr_t* fd, void* buf, uint32_t count)
{
    kql_acquire(fd->lock);

    if (!(fd->perms & FD_PERM_WRITE)) {
        panic("thread doesn't have the permission to write to file descriptor");
    }

    switch (fd->type) {
    case FD_TYPE_STREAM_DEV:
        kql_acquire(fd->dev->lock);
        bq_add(fd->dev->write_buf, buf, count);
        kql_release(fd->dev->lock);
        break;
    case FD_TYPE_PIPE:
        bq_add(fd->pipe, buf, count);
        break;
    default:
        panic("kwrite : unknown fd type");
    }

    kql_release(fd->lock);
}

int kread(file_descr_t* fd, void* buf, uint32_t count)
{
    kql_acquire(fd->lock);

    if (!(fd->perms & FD_PERM_READ)) {
        panic("thread doesn't have the permission to read from file descriptor");
    }

    switch (fd->type) {
    case FD_TYPE_STREAM_DEV:
        kql_acquire(fd->dev->lock);
        bq_remove(fd->dev->read_buf, buf, count);
        kql_release(fd->dev->lock);
        break;
    case FD_TYPE_PIPE:
        bq_remove(fd->pipe, buf, count);
        break;
    default:
        panic("kwrite : unknown fd type");
    }

    kql_release(fd->lock);
}

void kpipe(file_descr_t** from_ptr, file_descr_t** to_ptr)
{
    blocking_queue_t* pipe = bq_create(FD_PIPE_SIZE);
    file_descr_t* from = kmalloc(sizeof(file_descr_t));
    file_descr_t* to = kmalloc(sizeof(file_descr_t));
    from->type = kql_create();
    to->lock = kql_create();
    from->type = to->type = FD_TYPE_PIPE;
    from->perms = to->perms = 0;
    
    from->perms |= FD_PERM_WRITE;
    to->perms |= FD_PERM_READ;

    from->pipe = to->pipe = pipe;

    *from_ptr = from;
    *to_ptr = to;
}