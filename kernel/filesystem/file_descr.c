#include "filesystem/file_descr.h"
#include "memory/kheap.h"
#include <string.h>
#include <stdio.h>
#include <panic.h>
#include "drivers/dev.h"

file_descr_t* fd_copy(file_descr_t* fd)
{
    file_descr_t* copy = kmalloc(sizeof(file_descr_t));
    copy->lock = kql_create();
    copy->type = fd->type;
    copy->perms = fd->perms;
    switch (fd->type) {
    case FD_TYPE_FILE:
        copy->inode = fd->inode;
        copy->offset = fd->offset;
        break;
    case FD_TYPE_STREAM_DEV:
        copy->dev = fd->dev;
        break;
    case FD_TYPE_PIPE:
        copy->pipe = fd->pipe;
        break;
    default: 
        panic("unknown file descr type in fd_copy()");
        break;
    }
    return fd;
}

static bool is_dev_path(char* path)
{
    // don't compare with the null terminator, we only 
    // want to check for a prefix.
    return memcmp(path, DEV_FOLDER, strlen(DEV_FOLDER)) == 0;
}

static char* dev_name(char* path)
{
    // don't forget to remove the sedond '/'
    return path + strlen(DEV_FOLDER) + 1;
}

static file_descr_t* open_dev(char* name, uint8_t perms)
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
        if (dev->perms && FD_PERM_READ) {
            fd->perms |= FD_PERM_READ;
        }
        else {
            printf("can't read from device %s\n", name);
            panic("open\n");
        }
    }
    if (perms & FD_PERM_WRITE) {
        if (dev->perms && FD_PERM_WRITE) {
            fd->perms |= FD_PERM_WRITE;
        }
        else {
            printf("can't write to device %s\n", name);
            panic("open\n");
        }
    }
    if (perms & FD_PERM_SEEK) {
        if (dev->perms && FD_PERM_SEEK) {
            fd->perms |= FD_PERM_SEEK;
        }
        else {
            printf("can't seek in device %s\n", name);
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

void kseek(file_descr_t* fd, int ofs, uint8_t flags)
{
    if (!(fd->perms & FD_PERM_SEEK)) {
        panic("thread doesn't have the permission to seek in file descriptor");
    }

    switch (fd->type) {
    case FD_TYPE_STREAM_DEV:
        fd->dev->seek(ofs, flags);
        break;
    // we can never seek in a pipe
    default:
        panic("kseek : unknown fd type");
    }
}

int kwrite(file_descr_t* fd, void* buf, uint32_t count)
{
    kql_acquire(fd->lock);

    if (!(fd->perms & FD_PERM_WRITE)) {
        panic("thread doesn't have the permission to write to file descriptor");
    }

    int c;
    switch (fd->type) {
    case FD_TYPE_STREAM_DEV:
        c = fd->dev->write(buf, count);
        break;
    case FD_TYPE_PIPE:
        bq_add(fd->pipe, buf, count);
        c = count;
        break;
    default:
        panic("kwrite : unknown fd type");
    }

    kql_release(fd->lock);
    return c;
}

int kread(file_descr_t* fd, void* buf, uint32_t count)
{
    kql_acquire(fd->lock);

    if (!(fd->perms & FD_PERM_READ)) {
        panic("thread doesn't have the permission to read from file descriptor");
    }

    int c;
    switch (fd->type) {
    case FD_TYPE_STREAM_DEV:
        c = fd->dev->read(buf, count);
        break;
    case FD_TYPE_PIPE:
        bq_remove(fd->pipe, buf, count);
        c = count;
        break;
    default:
        panic("kwrite : unknown fd type");
    }

    kql_release(fd->lock);
    return c;
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