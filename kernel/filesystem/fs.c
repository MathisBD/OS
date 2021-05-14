#include "filesystem/fs_internal.h"
#include "filesystem/ext2/ext2.h"
#include <string.h>
#include "memory/kheap.h"
#include <stdio.h>
#include <panic.h>


// convert from an ext2 error to an fs error
int cvrt_err(int ext2_err) 
{
    switch (ext2_err) {
    case EXT2_ERR_INODE_EXIST: return FS_ERR_INODE_EXIST;
    case EXT2_ERR_DISK_READ: return FS_ERR_READ;
    case EXT2_ERR_DISK_WRITE: return FS_ERR_WRITE;
    case EXT2_ERR_NO_SPACE: return FS_ERR_NO_SPACE;
    case EXT2_ERR_INODE_TYPE: return FS_ERR_INODE_TYPE;
    case EXT2_ERR_FILE_BOUNDS: return FS_ERR_FILE_BOUNDS;
    default: return FS_ERR_INTERN_FAIL;
    }
}

uint32_t cvrt_inode_type(uint32_t ext2_type)
{
    switch(ext2_type) {
    case EXT2_INODE_TYPE_DIR: return FS_INODE_TYPE_DIR;
    case EXT2_INODE_TYPE_REG: return FS_INODE_TYPE_FILE;
    default: panic("cvrt_inode_type: unknown ext2 inode type\n");
    }
    return 0;
}

// it is allowed that entry==0
int free_ext2_entr(ext2_dir_entry_t* entry)
{
    if (entry != 0) {
        int r = free_ext2_entr(entry->next);
        if (r < 0) {
            return r;
        }
        kfree(entry->name);
        kfree(entry);
    }
    return 0;
}

bool valid_ext2_dir(ext2_dir_entry_t* entry)
{
    // the first entry must be the self reference '.'
    if (entry == 0 || memcmp(entry->name, ".", 2) != 0) {
        return false;
    }
    // the second entry must be the parent reference '..'
    if (entry->next == 0 || memcmp(entry->next->name, "..", 3) != 0) {
        return false;
    }
    return true;
}


int dir_find_child(uint32_t dir, const char* child_name, uint32_t* child)
{
    ext2_dir_entry_t* entry;
    int r = read_dir(dir, &entry);
    if (r < 0) {
        free_ext2_entr(entry);
        return cvrt_err(r);
    }
    if (!valid_ext2_dir(entry)) {
        free_ext2_entr(entry);
        return FS_ERR_INTERN_FAIL;
    }

    uint32_t len = strlen(child_name);
    for (ext2_dir_entry_t* curr = entry; curr != 0; curr = curr->next) {
        if (memcmp(curr->name, child_name, len+1) == 0) {
            *child = curr->inode;
            free_ext2_entr(entry);
            return 0;
        }
    }

    // we didn't find the child
    free_ext2_entr(entry);
    return FS_ERR_INODE_EXIST;
}

int dir_add_child(uint32_t dir, const char* child_name, uint32_t child)
{
    // read the dir contents
    ext2_dir_entry_t* entry;
    int r = read_dir(dir, &entry);
    if (r < 0) {
        free_ext2_entr(entry);
        return cvrt_err(r);
    }
    if (!valid_ext2_dir(entry)) {
        free_ext2_entr(entry);
        return FS_ERR_INTERN_FAIL;
    }
    // create a new directory entry
    ext2_dir_entry_t* new_entr = kmalloc(sizeof(ext2_dir_entry_t));
    uint32_t len = strlen(child_name);
    new_entr->name = kmalloc(len + 1);
    memcpy(new_entr->name, child_name, len);
    new_entr->name[len] = 0;
    new_entr->name_len = (uint8_t)len;
    new_entr->inode = child;
    new_entr->next = 0;
    // for some reason the first entries have to be . and ..
    // so we add entries at the end of the directory
    ext2_dir_entry_t* last = entry;
    while (last->next != 0) {
        last = last->next;
    }
    last->next = new_entr;
    // write back the dir contents
    r = write_dir(dir, entry);
    free_ext2_entr(entry);
    if (r < 0) {
        return cvrt_err(r);
    }
    return 0;
}

int dir_rem_child(uint32_t dir, const char* child_name)
{
    // read the contents
    ext2_dir_entry_t* entry;
    int r = read_dir(dir, &entry);
    if (r < 0) {
        free_ext2_entr(entry);
        return cvrt_err(r);
    }
    if (!valid_ext2_dir(entry)) {
        free_ext2_entr(entry);
        return FS_ERR_INTERN_FAIL;
    }

    uint32_t len = strlen(child_name);
    ext2_dir_entry_t* prev = entry;
    for (ext2_dir_entry_t* curr = entry; curr != 0; curr = curr->next) {
        if (memcmp(curr->name, child_name, len+1) == 0) {        
            // remove the child's dir entry
            if (prev == entry) {
                entry = curr->next;
            }
            else {
                prev->next = curr->next;
            }
            curr->next = 0;
            free_ext2_entr(curr);
            // write back the contents 
            r = write_dir(dir, entry);
            free_ext2_entr(entry);
            if (r < 0) {
                return cvrt_err(r);
            }
            return 0;
        }
    }
    // we didn't find the child
    free_ext2_entr(entry);
    return FS_ERR_INODE_EXIST;
}

dir_entry_t* convert_dir_entry(ext2_dir_entry_t* ext2_entry)
{
    if (ext2_entry == 0) {
        return 0;
    }
    dir_entry_t* entry = kmalloc(sizeof(dir_entry_t));
    entry->inode = ext2_entry->inode;
    entry->name = kmalloc(1 + (uint32_t)ext2_entry->name_len);
    memcpy(entry->name, ext2_entry->name, ext2_entry->name_len);
    entry->name[ext2_entry->name_len] = 0;
    entry->next = convert_dir_entry(ext2_entry->next);
    return entry;
}

int fs_list_dir(uint32_t dir, dir_entry_t** entries)
{
    // read the contents
    ext2_dir_entry_t* entry;
    int r = read_dir(dir, &entry);
    if (r < 0) {
        free_ext2_entr(entry);
        return cvrt_err(r);
    }
    if (!valid_ext2_dir(entry)) {
        free_ext2_entr(entry);
        return FS_ERR_INTERN_FAIL;
    }
    // convert the structs
    *entries = convert_dir_entry(entry);
    free_ext2_entr(entry);
    return 0;
}

// returns strlen(str) if the character wasn't found
// returns the index of the FIRST occurence otherwise
size_t find_char(const char* str, char c)
{
    size_t i = 0;
    while (str[i] != 0) {
        if (str[i] == c) {
            return i;
        }
        i++;
    }
    return i;
}

// returns strlen(str) if the character wasn't found
// returns the index of the LAST occurence otherwise
size_t find_char_rev(const char* str, char c)
{
    size_t len = strlen(str);
    size_t i = len;
    while (i > 0) {
        i--;
        if (str[i] == c) {
            return i;
        }
    }
    return len;
}

bool valid_path(const char* path)
{
    size_t len = strlen(path);
    if (path[0] != '/') {
        return false;
    }

    size_t i = 0; // points to a '/' or to the null terminator
    while (i < len) {
        size_t old_i = i;
        i = old_i + 1 + find_char(path + old_i + 1, '/');
        // can't have an empty dir/file name
        if (i == old_i + 1) {
            return false;        
        }
        // check dir/file name size is not too big
        if (i - old_i - 1 > FS_MAX_NAME_LEN) {
            return false;
        }
    }
    return true;
}

int fs_find_inode(const char* path, uint32_t* inode)
{
    if (!valid_path(path)) {
        return FS_ERR_PATH_FORMAT;
    }
    size_t path_len = strlen(path);
    // start and length of the current dir/file name in the path,
    // not including the '/' separators
    size_t start = 0;
    size_t len = 0;
    *inode = ROOT_INODE;
    while (start + len < path_len) {
        // find the child's name
        start = start + len + 1; // skip the '/'
        len = find_char(path + start, '/');
        char* child_name = kmalloc(len + 1);
        memcpy(child_name, path + start, len);
        child_name[len] = 0;
        // find the child's inode
        int r = dir_find_child(*inode, child_name, inode);
        kfree(child_name);
        if (r < 0) {
            return cvrt_err(r);
        }
    }
    return 0;
}

int make_inode(const char* path, uint32_t type, uint32_t* inode, uint32_t* parent)
{
    if (!valid_path(path)) {
        return FS_ERR_PATH_FORMAT;
    }
    // get the parent
    size_t sep = find_char_rev(path, '/');
    // special case : root
    if (sep == 0) {
        *parent = ROOT_INODE;
    }
    else {
        char* par_path = kmalloc(sep + 1);
        memcpy(par_path, path, sep);
        par_path[sep] = 0;
        int r = fs_find_inode(par_path, parent);
        kfree(par_path);
        if (r < 0) {
            return r;
        }
    }
    // create the file (or dir)
    char* inode_name = path + sep + 1;
    int r = alloc_inode(inode, type);
    if (r < 0) {
        return cvrt_err(r);
    }
    // link with the parent
    r = dir_add_child(*parent, inode_name, *inode);
    if (r < 0) {
        // don't forget to free the inode
        int r2 = free_inode(*inode);
        // double failure
        if (r2 < 0) {
            return FS_ERR_INTERN_FAIL;
        }
        return cvrt_err(r);
    }
    return 0;
}

int fs_make_file(const char* path)
{
    uint32_t inode;
    uint32_t parent;
    return make_inode(path, FS_INODE_TYPE_FILE, &inode, &parent);
}

int fs_make_dir(const char* path)
{
    uint32_t inode;
    uint32_t parent;
    int r = make_inode(path, FS_INODE_TYPE_DIR, &inode, &parent);
    if (r < 0) {
        return r;
    }
    // self reference '.'
    r = dir_add_child(inode, ".", inode);
    if (r < 0) {
        return r;
    }
    // parent reference '..'
    r = dir_add_child(inode, "..", parent);
    if (r < 0) {
        return r;
    }
    return 0;
}

int rem_inode(const char* path, uint32_t type)
{
    if (!valid_path(path)) {
        return FS_ERR_PATH_FORMAT;
    }
    // get the parent
    size_t sep = find_char_rev(path, '/');
    uint32_t parent;
    // special case : root
    if (sep == 0) {
        parent = ROOT_INODE;
    }
    else {
        char* par_path = kmalloc(sep + 1);
        memcpy(par_path, path, sep);
        par_path[sep] = 0;
        int r = fs_find_inode(par_path, &parent);
        kfree(par_path);
        if (r < 0) {
            return r;
        }
    }
    // get the inode
    char* inode_name = path + sep + 1;
    uint32_t inode;
    int r = dir_find_child(parent, inode_name, &inode);
    if (r < 0) {
        return r;
    }
    // get the inode type
    uint32_t real_type;
    r = get_inode_type(inode, &real_type);
    if (r < 0) {
        return cvrt_err(r);
    }
    real_type = cvrt_inode_type(real_type);
    // check the type
    if (type != real_type) {
        return FS_ERR_INODE_TYPE;
    }
    // if directory, check it is empty
    if (type == FS_INODE_TYPE_DIR) {
        ext2_dir_entry_t* entry;
        r = read_dir(inode, &entry);
        if (r < 0) {
            free_ext2_entr(entry);
            return cvrt_err(r);
        }
        if (!valid_ext2_dir(entry)) {
            free_ext2_entr(entry);
            return FS_ERR_INTERN_FAIL;
        }
        if (entry->next->next != 0) {
            free_ext2_entr(entry);
            return FS_ERR_DIR_NONEMPTY;
        }
        free_ext2_entr(entry);
    }
    // remove the inode from its parent
    r = dir_rem_child(parent, inode_name);
    if (r < 0) {
       return r;
    }
    // free the inode
    r = resize_inode(inode, 0);
    if (r < 0) {
        return cvrt_err(r);
    }
    r = free_inode(inode);
    if (r < 0) {
        return cvrt_err(r);
    }
    return 0;
}


int fs_rem_file(const char* path)
{
    return rem_inode(path, FS_INODE_TYPE_FILE);
}

int fs_rem_dir(const char* path)
{
    return rem_inode(path, FS_INODE_TYPE_DIR);
}

int fs_read_file(uint32_t file, uint32_t offset, uint32_t count, void* buf)
{
    // check inode type (this automatically checks the inode exists)
    uint32_t type;
    int r = get_inode_type(file, &type);
    if (r < 0) {
        return cvrt_err(r);
    }
    if (type != EXT2_INODE_TYPE_REG) {
        return FS_ERR_INODE_TYPE;
    }
    // read (this automatically checks file bounds)
    r = read_inode(file, offset, count, buf);
    if (r < 0) {
        return cvrt_err(r);
    }
    return 0;
}

int fs_write_file(uint32_t file, uint32_t offset, uint32_t count, void* buf)
{
    // check inode type (this automatically checks the inode exists)
    uint32_t type;
    int r = get_inode_type(file, &type);
    if (r < 0) {
        return cvrt_err(r);
    }
    if (type != EXT2_INODE_TYPE_REG) {
        return FS_ERR_INODE_TYPE;
    }
    // grow the file if needed
    uint32_t fsize;
    r = get_inode_fsize(file, &fsize);
    if (r < 0) {
        return cvrt_err(r);
    }
    if (offset + count > fsize) {
        r = resize_inode(file, offset + count);
        if (r < 0) {
            return cvrt_err(r);
        }
    }
    // write
    r = write_inode(file, offset, count, buf);
    if (r < 0) {
        return cvrt_err(r);
    }
    return 0;
}

int fs_resize_file(uint32_t file, uint32_t size)
{
    // check inode type (this automatically checks the inode exists)
    uint32_t type;
    int r = get_inode_type(file, &type);
    if (r < 0) {
        return cvrt_err(r);
    }
    if (type != EXT2_INODE_TYPE_REG) {
        return FS_ERR_INODE_TYPE;
    }
    // resize
    r = resize_inode(file, size);
    if (r < 0) {
        return cvrt_err(r);
    }
    return 0;
}

int fs_file_size(uint32_t file, uint32_t* size)
{
    // check inode type (this automatically checks the inode exists)
    uint32_t type;
    int r = get_inode_type(file, &type);
    if (r < 0) {
        return cvrt_err(r);
    }
    if (type != EXT2_INODE_TYPE_REG) {
        return FS_ERR_INODE_TYPE;
    }
    // get the size
    r = get_inode_fsize(file, size);
    if (r < 0) {
        return cvrt_err(r);
    }
    return 0;
}


int fs_inode_type(uint32_t inode, uint32_t* type)
{
    int r = get_inode_type(inode, type);
    if (r < 0) {
        return cvrt_err(r);
    }
    *type = cvrt_inode_type(*type);
    return 0;
}

int init_fs()
{
    int r = init_ext2();
    if (r < 0) {
        return cvrt_err(r);
    }
    return 0;
}