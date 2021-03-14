// Ext2 directory handling.

#include "filesystem/ext2/ext2_internal.h"
#include "memory/heap.h"
#include <string.h>


void decode_dir_entry(void* contents, dir_entry_t* dir_entry, uint32_t* size)
{
    dir_entry->inode = *((uint32_t*)(contents + 0));
    *size = *((uint16_t*)(contents + 4));
    dir_entry->name_len = 0x00FF & *((uint16_t*)(contents + 6));
    dir_entry->name = malloc(dir_entry->name_len);
    memcpy(dir_entry->name, contents + 8, dir_entry->name_len);   
}

int dir_list(uint32_t dir_num, dir_entry_t** entries)
{
    // read the whole contents of the directory
    uint32_t size;
    int r = inode_fsize(dir_num, &size);
    if (r < 0) {
        return r;
    }
    if (size == 0) {
        *entries = 0;
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
    decode_dir_entry(contents, *entries, entr_size);
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

