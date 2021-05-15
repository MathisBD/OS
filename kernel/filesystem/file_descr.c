#include "filesystem/file_descr.h"
#include "memory/kheap.h"
#include <string.h>
#include <stdio.h>
#include <panic.h>
#include "drivers/dev.h"
#include "filesystem/fs.h"

file_descr_t* fd_copy(file_descr_t* fd)
{
    file_descr_t* copy = kmalloc(sizeof(file_descr_t));
    copy->lock = kql_create();
    copy->type = fd->type;
    copy->perms = fd->perms;
    switch (fd->type) {
    case FD_TYPE_FILE:
        copy->finode = fd->finode;
        copy->offset = fd->offset;
        break;
    case FD_TYPE_DIR:
        copy->dinode = fd->dinode;
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
    // file/dir
    uint32_t inode;
    int r = fs_find_inode(path, &inode);
    if (r < 0) {
        printf("kopen : couldn't find file/dir %s\n", path);
        while(1);
    }
    uint32_t ftype;
    r = fs_inode_type(inode, &ftype);
    if (r < 0) {
        printf("kopen : couldn't determine type of inode %s\n", path);
        while(1);
    }

    file_descr_t* fd = kmalloc(sizeof(file_descr_t));
    fd->lock = kql_create();
    fd->perms = perms & (FD_PERM_READ | FD_PERM_WRITE | FD_PERM_SEEK);
    switch (ftype) {
    case FS_INODE_TYPE_FILE: 
        fd->type = FD_TYPE_FILE; 
        fd->finode = inode;
        fd->offset = 0;
        break;
    case FS_INODE_TYPE_DIR: 
        fd->type = FD_TYPE_DIR; 
        fd->dinode = inode;
        break;
    default: panic("kopen"); break;
    }
    
    return fd;
}

void kclose(file_descr_t* fd)
{
    kql_acquire(fd->lock);
    if (fd->type == FD_TYPE_PIPE) {
        // TODO : close the other end of the pipe
        panic("kclose : TODO pipe");
    }    
    //kql_delete(fd->lock);
    kfree(fd);
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
    case FD_TYPE_FILE:
        switch (flags) {
        case FD_SEEK_SET: fd->offset = ofs; break;
        case FD_SEEK_CUR: fd->offset += ofs; break;
        case FD_SEEK_END:
        {
            uint32_t size;
            int r = fs_file_size(fd->finode, &size);
            if (r < 0) {
                panic("kseek : couldn't get size of file");
            }
            fd->offset = size + ofs;
            break;
        }
        default:
            panic("kseek : unknown seek type");
        }
        break;
    // we can never seek in a pipe/dir
    default:
        panic("kseek : unknown fd type");
    }
}

uint32_t kfile_type(char* path)
{
    // device
    if (is_dev_path(path)) {
        char* name = dev_name(path);
        stream_dev_t* dev = get_stream_dev(name);
        if (dev != 0) {
            return FD_TYPE_STREAM_DEV;
        }
        return FD_TYPE_ERR;
    }
    // file/dir
    uint32_t inode;
    if (fs_find_inode(path, &inode) >= 0) {
        uint32_t type;
        if (fs_inode_type(inode, &type) >= 0) {
            switch (type) {
            case FS_INODE_TYPE_FILE: return FD_TYPE_FILE;
            case FS_INODE_TYPE_DIR: return FD_TYPE_DIR;
            default: panic("kfile_type\n");
            }
        }
        panic("kfile_type\n");
    }
    return FD_TYPE_ERR;
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
    case FD_TYPE_FILE:
    {
        int r = fs_write_file(fd->finode, fd->offset, count, buf);
        if (r < 0) {
            panic("kwrite");
        }
        c = count;
        break;
    }
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
    case FD_TYPE_FILE:
    {
        int r = fs_read_file(fd->finode, fd->offset, count, buf);
        if (r < 0) {
            panic("kread");
        }
        c = count;
        break;
    }
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


void kcreate(char* path, uint8_t type)
{
    switch (type) {
    case FD_TYPE_FILE:
    {
        int r = fs_make_file(path);
        if (r < 0) {
            printf("couldn't make file %s\n", path);
            while(1);
        }
        break;
    }
    case FD_TYPE_DIR:
    {
        int r = fs_make_dir(path);
        if (r < 0) {
            printf("couldn't make directory %s\n", path);
            while(1);
        }
        break;
    }
    default:
    {
        panic("kcreate : unknown file type");
        break;
    }
    }
}

void kremove(char* path, uint8_t type)
{   
    switch (type) {
    case FD_TYPE_FILE:
    {
        int r = fs_rem_file(path);
        if (r < 0) {
            printf("couldn't remove file %s\n", path);
            while(1);
        }
        break;
    }
    case FD_TYPE_DIR:
    {
        int r = fs_rem_dir(path);
        if (r < 0) {
            printf("couldn't remove directory %s\n", path);
            while(1);
        }
        break;
    }
    default:
    {
        panic("kremove : unknown file type");
        break;
    }
    }
}

uint32_t kget_size(file_descr_t* fd)
{
    kql_acquire(fd->lock);
    uint32_t size;
    switch (fd->type) {
    case FD_TYPE_FILE:
    {
        int r = fs_file_size(fd->finode, &size);
        if (r < 0) {
            panic("kget_size");
            while(1);
        }
        break;
    }
    default:
    {
        panic("kcreate : unknown file type");
        size = 0;
        break;
    }
    }
    kql_release(fd->lock);
    return size;
}

void kresize(file_descr_t* fd, uint32_t size)
{
    kql_acquire(fd->lock);

    uint32_t old_size;
    switch (fd->type) {
    case FD_TYPE_FILE:
    {
        int r = fs_resize_file(fd->finode, size);
        if (r < 0) {
            panic("kresize");
            while(1);
        }
        break;
    }
    default:
    {
        panic("kcreate : unknown file type");
        break;
    }
    }
    kql_release(fd->lock);
}

int klist_dir(file_descr_t* fd, void* buf, uint32_t size)
{
    kql_acquire(fd->lock);
    switch (fd->type) {
    case FD_TYPE_DIR:
    {
        dir_entry_t* ent;
        int r = fs_list_dir(fd->dinode, &ent);
        if (r < 0) {
            panic("klist_dir");
        }
        uint32_t i = 0;
        while (ent != 0) {
            uint32_t len = strlen(ent->name) + 1;
            if (i + len > size) {
                kql_release(fd->lock);
                return i;
            }
            memcpy(buf + i, ent->name, len);
            i += len;
            ent = ent->next; 
        }
        break;
    }
    default:
    {
        panic("klist_dir : unknown fd type");
        break;
    }
    }
    kql_release(fd->lock);
    return 0;
}
