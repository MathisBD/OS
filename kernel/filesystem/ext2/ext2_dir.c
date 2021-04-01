// Ext2 directory handling.

#include "filesystem/ext2/ext2_internal.h"
#include "memory/kheap.h"
#include <string.h>

// align val, rounding upwards
// assumes 'align' is a power of 2 and val is unsigned
#define ALIGN(val, align) \
    ((val) & ((align)-1)) ? \
    (((val) & ~((align)-1)) + (align)) : \
    (val) 
    

void decode_dir_entry(void* contents, ext2_dir_entry_t* dir_entry, uint32_t* size)
{
    dir_entry->inode = *((uint32_t*)(contents + 0));
    *size = *((uint16_t*)(contents + 4));
    dir_entry->name_len = *((uint16_t*)(contents + 6));
    dir_entry->name = kmalloc(1 + dir_entry->name_len); // don't forget null terminator
    memcpy(dir_entry->name, contents + 8, dir_entry->name_len);   
    dir_entry->name[dir_entry->name_len] = 0;
}


int read_dir(uint32_t dir_num, ext2_dir_entry_t** entries)
{
    // check it is a directory
    uint32_t type;
    int r = get_inode_type(dir_num, &type);
    if (r < 0) {
        return r;
    }
    if (type != EXT2_INODE_TYPE_DIR) {
        return EXT2_ERR_INODE_TYPE;
    }
    // read the whole contents of the directory
    uint32_t size;
    r = get_inode_fsize(dir_num, &size);
    if (r < 0) {
        return r;
    }
    if (size == 0) {
        *entries = 0;
        return 0;
    }
    void* contents = kmalloc(size);
    r = read_inode(dir_num, 0, size, contents);
    if (r < 0) {
        kfree(contents);
        return r;
    }

    // parse the contents and populate the output buffer
    uint32_t ofs = 0;
    uint32_t entr_size;
    // first entry separately
    *entries = kmalloc(sizeof(ext2_dir_entry_t));
    decode_dir_entry(contents, *entries, &entr_size);
    ofs += entr_size;
    // other entries
    ext2_dir_entry_t* curr = *entries;
    while (ofs < size) {
        curr->next = kmalloc(sizeof(ext2_dir_entry_t));
        uint32_t entr_size;
        decode_dir_entry(contents + ofs, curr->next, &entr_size);
        ofs += entr_size;
        curr = curr->next;
    }
    kfree(contents);
    curr->next = 0;
    if (ofs > size) {
        return EXT2_ERR_CORRUPT_STATE;
    }
    return 0;
}

// offset : offset in the file the entry will be written at
// assumes the entry can fit in the current block
uint16_t entry_size(ext2_dir_entry_t* entr, uint32_t offset)
{
    // disk size of this entry is 8 bytes + name_len : 
    // 4 (inode number) + 2 (entry size) + 1 (name length) + 1 (type indicator) 
    uint16_t size = 8 + (uint16_t)(entr->name_len);
    size = ALIGN(size, 4);
    if (entr->next == 0) {
        // the last entry extends to the end of the block
        uint32_t bl_end = ALIGN(offset + size, sb->block_size);
        size = bl_end - offset;
    }
    else {
        uint16_t next_size = 8 + (uint16_t)(entr->next->name_len);
        next_size = ALIGN(next_size, 4);
        uint32_t bl_end = ALIGN(offset + size, sb->block_size);
        // the next entry mustn't span multiple blocks
        if (offset + size + next_size > bl_end) {
            size = bl_end - offset;
        }
    }
    return size;
}

int write_dir(uint32_t dir_num, ext2_dir_entry_t* entries)
{
    // check it is a directory
    uint32_t type;
    int r = get_inode_type(dir_num, &type);
    if (r < 0) {
        return r;
    }
    if (type != EXT2_INODE_TYPE_DIR) {
        return EXT2_ERR_INODE_TYPE;
    }
    // serialize the entry list
    uint32_t size = 0;
    for (ext2_dir_entry_t* entr = entries; entr != 0; entr = entr->next) {
        size += entry_size(entr, size);
    }
    void* contents = kmalloc(size);
    // we zero out the bytes we don't use, because why not ?
    memset(contents, 0, size);
    uint32_t ofs = 0;
    for (ext2_dir_entry_t* entr = entries; entr != 0; entr = entr->next) {
        uint16_t e_size = entry_size(entr, ofs);
        *((uint32_t*)(contents + ofs + 0)) = entr->inode;
        *((uint16_t*)(contents + ofs + 4)) = e_size;
        *((uint16_t*)(contents + ofs + 6)) = entr->name_len;
        memcpy(contents + ofs + 8, entr->name, entr->name_len);
        ofs += e_size;
    }
    // resize the inode
    r = resize_inode(dir_num, size );
    if (r < 0) {
        kfree(contents);
        return r;
    }
    // write the contents
    r = write_inode(dir_num, 0, size, contents);
    kfree(contents);
    return r;
}