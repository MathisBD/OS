// Ext2 inode allocation/resizing.

#include "filesystem/ext2/ext2_internal.h"
#include <string.h>
#include <bitset.h>
#include <stdbool.h>


// finds a free inode and allocates it
// the found inode number is passed back to the caller
// via inode_num
int claim_new_inode(uint32_t* inode_num)
{
    if (sb->unalloc_inodes == 0) {
        return ERR_NO_SPACE;
    }

    int r;
    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    uint32_t bg_num = 0;
    while (bg_num < sb->bg_count) {
        r = get_bg_descr(bg_num, bg);
        if (r < 0) {
            free(bg);
            return r;
        }
        // we found the block group
        if (bg->unalloc_inodes > 0) {
            break;
        }
        bg_num++;
    }
    if (bg_num >= sb->bg_count) {
        free(bg);
        return ERR_CORRUPT_STATE;
    }

    // read the bitmap
    void* bitmap = malloc(sb->block_size);
    r = read_block(bg->inode_bitmap, 0, sb->block_size, bitmap);
    if (r < 0) {
        free(bitmap);
        free(bg);
        return r;
    }

    // find the first unallocated inode in the block group
    uint32_t idx = bitset_find_zero(bitmap, sb->inodes_per_bg);
    if (idx >= sb->inodes_per_bg) {
        free(bitmap);
        free(bg);
        return ERR_CORRUPT_STATE;
    }

    // actually claim the inode
    *inode_num = bg_num * sb->inodes_per_bg + idx + 1;
    bitset_set(bitmap, idx);
    bg->unalloc_inodes--;
    sb->unalloc_inodes--;

    // write back to disk
    r = write_block(bg->inode_bitmap, 0, sb->block_size, bitmap);
    if (r < 0) {
        free(bitmap);
        free(bg);
        return r;
    }
    r = sync_bg_descr(bg_num, bg);
    if (r < 0) {
        free(bitmap);
        free(bg);
        return r;
    }
    r = sync_superblock(sb);
    if (r < 0) {
        free(bitmap);
        free(bg);
        return r;
    }

    free(bitmap);
    free(bg);
    return 0;
}

// set a clean state for a newly allocated inode
int init_inode(inode_t* inode, uint32_t type)
{
    inode->fsize = 0;
    inode->type = type;

    memset(inode->dir_blocks, 0, INODE_DIR_BLOCKS);
    inode->single_indir = 0;
    inode->double_indir = 0;

    return 0;
}

int alloc_inode(uint32_t* inode_num, uint32_t type)
{
    // deal with the "occupancy" state
    int r = claim_new_inode(inode_num);
    if (r < 0) {
        return r;
    }

    // deal with the inode state
    inode_t* inode = malloc(sizeof(inode_t));    
    r = init_inode(inode, type);
    if (r < 0) {
        free(inode);
        return r;
    }
    r = sync_inode(*inode_num, inode);
    free(inode);
    return r;
}


int free_inode(uint32_t inode_num)
{
    uint32_t bg_num = (inode_num - 1) / sb->inodes_per_bg;
    uint32_t idx = (inode_num - 1) % sb->inodes_per_bg;

    // get the block group descriptor
    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    int r = get_bg_descr(bg_num, bg);
    if (r < 0) {
        free(bg);
        return r;
    }

    // read the bitmap
    void* bitmap = malloc(sb->block_size);
    r = read_block(bg->inode_bitmap, 0, sb->block_size, bitmap);
    if (r < 0) {
        free(bitmap);
        free(bg);
        return r;
    }

    // clear the inode's corresponding bit
    // and write back
    bitset_clear(bitmap, idx);
    r = write_block(bg->inode_bitmap, 0, sb->block_size, bitmap);
    free(bitmap);
    if (r < 0) {
        free(bg);
        return r;
    }

    // update the block group descriptor
    bg->unalloc_inodes++;
    r = sync_bg_descr(bg_num, bg);
    free(bg);
    if (r < 0) {
        return r;
    }

    // update the superblock
    sb->unalloc_inodes++;
    r = sync_superblock(sb);
    return r;
}


// finds a free block and allocates it
// we try to allocate the new block near prev_block
// or even (jackpot) right after prev_block
int claim_new_block(uint32_t* block, uint32_t prev_block)
{

}


int free_block(uint32_t block)
{
    
}