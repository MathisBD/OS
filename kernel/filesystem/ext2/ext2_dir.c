// Ext2 directory handling.

#include "filesystem/ext2/ext2_internal.h"
#include "memory/heap.h"
#include <string.h>


void decode_dir_entry(void* contents, dir_entry_t* dir_entry, uint32_t* size)
{
    dir_entry->inode = *((uint32_t*)(contents + 0));
    *size = *((uint16_t*)(contents + 4));
    dir_entry->name_len = 0x00FF & *((uint16_t*)(contents + 6));
    dir_entry->name = malloc(1 + dir_entry->name_len); // don't forget null terminator
    memcpy(dir_entry->name, contents + 8, dir_entry->name_len);   
    dir_entry->name[dir_entry->name_len] = 0;
}


int read_dir(uint32_t dir_num, dir_entry_t** entries)
{
    // check it is a directory
    uint32_t type;
    int r = inode_type(dir_num, &type);
    if (r < 0) {
        return r;
    }
    if (type != INODE_TYPE_DIR) {
        return ERR_INODE_TYPE;
    }
    // read the whole contents of the directory
    uint32_t size;
    r = inode_fsize(dir_num, &size);
    if (r < 0) {
        return r;
    }
    if (size == 0) {
        *entries = 0;
        return 0;
    }
    void* contents = malloc(size);
    r = read_inode(dir_num, 0, size, contents);
    if (r < 0) {
        free(contents);
        return r;
    }

    // parse the contents and populate the output buffer
    uint32_t ofs = 0;
    uint32_t entr_size;
    // first entry separately
    *entries = malloc(sizeof(dir_entry_t));
    decode_dir_entry(contents, *entries, &entr_size);
    ofs += entr_size;
    // other entries
    dir_entry_t* curr = *entries;
    while (ofs < size) {
        curr->next = malloc(sizeof(dir_entry_t));
        uint32_t entr_size;
        decode_dir_entry(contents + ofs, curr->next, &entr_size);
        ofs += entr_size;
        curr = curr->next;
    }
    free(contents);
    curr->next = 0;
    if (ofs > size) {
        return ERR_CORRUPT_STATE;
    }
    return 0;
}

uint16_t entry_size(dir_entry_t* entr)
{
    // disk size of this entry is 8 bytes + name_len : 
    // 4 (inode number) + 2 (entry size) + 1 (name length) + 1 (type indicator) 
    uint16_t size = 8 + (uint16_t)entr->name_len;
    // align the size on 4 bytes
    uint16_t align = 4;
    if (size & (align-1)) {
        size &= align;
        size += align;
    }
    return size;
}

int write_dir(uint32_t dir_num, dir_entry_t* entries)
{
    // check it is a directory
    uint32_t type;
    int r = inode_type(dir_num, &type);
    if (r < 0) {
        return r;
    }
    if (type != INODE_TYPE_DIR) {
        return ERR_INODE_TYPE;
    }
    // serialize the entry list
    uint32_t size = 0;
    for (dir_entry_t* entr = entries; entr != 0; entr = entr->next) {
        size += entry_size(entr);
    }
    void* contents = malloc(size);
    uint32_t ofs = 0;
    for (dir_entry_t* entr = entries; entr != 0; entr = entr->next) {
        *((uint32_t*)(contents + ofs + 0)) = entr->inode;
        *((uint16_t*)(contents + ofs + 4)) = entry_size(entr);
        *((uint8_t*)(contents + ofs + 6)) = entr->name_len;
        *((uint8_t*)(contents + ofs + 7)) = 0;
        memcpy(contents + ofs + 8, entr->name, entr->name_len);
        ofs += entry_size(entr);
    }
    // resize the inode
    r = resize_inode(dir_num, size);
    if (r < 0) {
        free(contents);
        return r;
    }
    // write the contents
    r = write_inode(dir_num, 0, size, contents);
    free(contents);
    return r;
}