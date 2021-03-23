// Ext2 inode allocation/resizing.

#include "filesystem/ext2/ext2_internal.h"
#include <string.h>
#include <bitset.h>
#include <stdbool.h>
#include <panic.h>
#include "memory/heap.h"

// the number of blocks we search before and after
// the previous block when allocating a new block
#define BLOCK_ALLOC_WINDOW  32


// finds a free inode and allocates it
// the found inode number is passed back to the caller
// via inode_num
int claim_new_inode(uint32_t* inode_num)
{
    if (sb->unalloc_inodes == 0) {
        return EXT2_ERR_NO_SPACE;
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
        return EXT2_ERR_CORRUPT_STATE;
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
        return EXT2_ERR_CORRUPT_STATE;
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

// returns 0 if there is no 
// unalloced block in the block group
// return >0 if we managed to allocate a block
int alloc_in_bg(uint32_t* block, uint32_t bg_num)
{
    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    int r = get_bg_descr(bg_num, bg);
    if (r < 0) {
        free(bg);
        return r;
    }
    if (bg->unalloc_blocks == 0) {
        return 0;
    }
    // read the bitmap
    void* bitmap = malloc(sb->block_size);
    r = read_block(bg->block_bitmap, 0, sb->block_size, bitmap);
    if (r < 0) {
        free(bg);
        free(bitmap);
        return r;
    }
    // allocate
    uint32_t idx = bitset_find_zero(bitmap, sb->blocks_per_bg);
    bitset_set(bitmap, idx);
    bg->unalloc_blocks--;
    *block = idx + bg_num * sb->blocks_per_bg;
    // write back the bitmap
    r = write_block(bg->block_bitmap, 0, sb->block_size, bitmap);
    if (r < 0) {
        free(bg);
        free(bitmap);
        return r;
    }
    // write back the block group descriptor
    r = sync_bg_descr(bg_num, bg);
    free(bitmap);
    free(bg);
    return r;
}

int alloc_block(uint32_t* blocks, uint32_t bg_num)
{
    if (bg_num >= sb->bg_count) {
        return EXT2_ERR_BG_EXIST;
    }
    if (sb->unalloc_blocks == 0) {
        return EXT2_ERR_NO_SPACE;
    }
    // in preferred block group
    int r = alloc_in_bg(blocks, bg_num);
    if (r < 0) { return r; }
    if (r > 0) { goto alloc_block_success; }
    // anywhere
    for (uint32_t bg_num = 0; bg_num < sb->bg_count; bg_num++) {
        r = alloc_in_bg(blocks, bg_num);
        if (r < 0) { return r; }
        if (r > 0) { goto alloc_block_success; } 
    }
alloc_block_failure:
    // this should not happen
    return EXT2_ERR_CORRUPT_STATE;

alloc_block_success:
    // update the superblock
    sb->unalloc_blocks--;
    r = sync_superblock(sb);
    return r;
}


int free_block(uint32_t block_num)
{
    // nothing to do for sparse blocks
    // (and block 0 can never be freed)
    if (block_num == 0) {
        return 0;
    }

    uint32_t bg_num = block_num / sb->blocks_per_bg;
    uint32_t idx = block_num % sb->blocks_per_bg;

    // get the block group descriptor
    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    int r = get_bg_descr(bg_num, bg);
    if (r < 0) {
        free(bg);
        return r;
    }

    // read the bitmap
    void* bitmap = malloc(sb->block_size);
    r = read_block(bg->block_bitmap, 0, sb->block_size, bitmap);
    if (r < 0) {
        free(bitmap);
        free(bg);
        return r;
    }

    // clear the block's corresponding bit
    // and write back
    bitset_clear(bitmap, idx);
    r = write_block(bg->block_bitmap, 0, sb->block_size, bitmap);
    free(bitmap);
    if (r < 0) {
        free(bg);
        return r;
    }

    // update the block group descriptor
    bg->unalloc_blocks++;
    r = sync_bg_descr(bg_num, bg);
    free(bg);
    if (r < 0) {
        return r;
    }

    // update the superblock
    sb->unalloc_blocks++;
    r = sync_superblock(sb);
    return r;
}

uint32_t div_ceil(uint32_t numer, uint32_t denom)
{
    uint32_t quot = numer / denom;
    if (numer % denom > 0) {
        return quot + 1;
    }
    return quot;
}

int resize_inode(uint32_t inode_num, uint32_t size)
{
    uint32_t old_size;
    int r = get_inode_fsize(inode_num, &old_size);
    if (r < 0) {
        return r;
    }
    uint32_t curr_blocks = div_ceil(old_size, sb->block_size);
    uint32_t new_blocks = div_ceil(size, sb->block_size);

    // free blocks
    if (new_blocks < curr_blocks) {
        uint32_t bl_count = curr_blocks - new_blocks;
        uint32_t* bl_nums = malloc(bl_count * sizeof(uint32_t));
        // get the block numbers
        r = read_bl_nums(inode_num, new_blocks, bl_count, bl_nums);
        if (r < 0) {
            free(bl_nums);
            return r;
        }

        for (uint32_t i = 0; i < bl_count; i++) {
            if (bl_nums[i] != 0) {
                // free the block
                r = free_block(bl_nums[i]);
                if (r < 0) {
                    free(bl_nums);
                    return r;
                }
                // remove it from the inode's blocks
                // (this should free intermediate blocks if
                // they become sparse)
                printf("b%u ", new_blocks+i);
                r = write_bl_num(inode_num, new_blocks+i, 0);
                if (r < 0) {
                    free(bl_nums);
                    return r;
                }
            }
        }
        free(bl_nums);
    }
    // increase size with sparse blocks
    else if (new_blocks > curr_blocks) {
        // get the inode
        inode_t* inode = malloc(sizeof(inode_t));
        int r = get_inode(inode_num, inode);
        if (r < 0) {
            free(inode);
            return r;
        }
        for (uint32_t i = curr_blocks; i < INODE_DIR_BLOCKS && i < new_blocks; i++) {
            inode->dir_blocks[i] = 0;
        }
        uint32_t c = INODE_DIR_BLOCKS;
        if (curr_blocks <= c && new_blocks > c) {
            inode->single_indir = 0;
        }
        c += sb->block_size / sizeof(uint32_t);
        if (curr_blocks <= c && new_blocks > c) {
            inode->double_indir = 0;
        }
        // sync the inode
        r = sync_inode(inode_num, inode);
        free(inode);
        if (r < 0) {
            return r;
        }
    }
    // update the inode's fsize
    r = set_inode_fsize(inode_num, size);
    return r;
}


