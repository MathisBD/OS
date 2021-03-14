// Ext2 inode allocation/resizing.

#include "filesystem/ext2/ext2_internal.h"
#include <string.h>
#include <bitset.h>
#include <stdbool.h>

// the number of blocks we search before and after
// the previous block when allocating a new block
#define BLOCK_ALLOC_WINDOW  32


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

// count : total number of blocks to allocated
// alloced : number of blocks allocated up to now 
// we have to update alloced
int alloc_near_bl(uint32_t* blocks, uint32_t count, uint32_t prev_bl, uint32_t* alloced)
{
    if (*alloced >= count) {
        return 0;
    }

    uint32_t prev_bg = prev_bl / sb->blocks_per_bg;
    uint32_t prev_idx = prev_bl % sb->blocks_per_bg;

    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    int r = get_bg_descr(prev_bg, bg);
    if (r < 0) { 
        free(bg);
        return r; 
    }

    // read the bitmap
    void* bitmap = malloc(sb->block_size);
    r = read_block(bg->block_bitmap, 0, sb->block_size, bitmap);
    if (r < 0) {
        free(bg);
        free(bitmap);
        return r;
    }

    // alloc right after the previous block
    uint32_t idx = prev_idx + 1;
    while (*alloced < count && idx < sb->blocks_per_bg && !bitset_test(bitmap, idx)) {
        bitset_set(bitmap, idx);
        blocks[*alloced] = idx + prev_bg * sb->blocks_per_bg;
        bg->unalloc_blocks--;
        *alloced += 1;
        idx += 1;
    }

    // alloc in a window around the previous block
    idx = (prev_idx > BLOCK_ALLOC_WINDOW) ? (prev_idx - BLOCK_ALLOC_WINDOW) : 0;
    while (*alloced < count && idx < sb->blocks_per_bg && idx < prev_idx + BLOCK_ALLOC_WINDOW) {
        if (!bitset_test(bitmap, idx)) {
            bitset_set(bitmap, idx);
            blocks[*alloced] = idx + prev_bg * sb->blocks_per_bg;
            bg->unalloc_blocks--;
            *alloced += 1;
        }
        idx++;
    }

    // write back the bitmap
    r = write_block(bg->block_bitmap, 0, sb->block_size, bitmap);
    free(bitmap);
    if (r < 0) {
        free(bg);
        return r;
    }
    // write back the block group descriptor
    r = sync_bg_descr(prev_bg, bg);
    free(bg);
    return r;
}

int alloc_in_bg(uint32_t* blocks, uint32_t count, uint32_t bg_num, uint32_t* alloced)
{
    if (*alloced >= count) {
        return 0;
    }

    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    int r = get_bg_descr(bg_num, bg);
    if (r < 0) {
        free(bg);
        return r;
    }
    if (bg->unalloc_blocks == 0) {
        return;
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
    uint32_t idx = 0;
    while (*alloced < count && bg->unalloc_blocks > 0) {
        uint32_t ofs = idx / 8; // we don't want to search through the whole bitmap again
        idx = 8*ofs + bitset_find_zero(bitmap + ofs, sb->blocks_per_bg - 8*ofs);
        bitset_set(bitmap, idx);
        bg->unalloc_blocks--;
        blocks[*alloced] = idx + bg_num * sb->blocks_per_bg;
        *alloced += 1;
    }
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

// finds free blocks and allocates them
// we try to allocate the new blocks near prev_block
// or even (jackpot) right after prev_block.
// if allocating for an empty inode,
// use prev_bl=inode_bg*blocks_per_bg (=first block of inode block group).
int alloc_blocks(uint32_t* blocks, uint32_t count, uint32_t prev_bl)
{
    if (prev_bl >= sb->block_count) {
        return ERR_BLOCK_EXIST;
    }
    if (sb->unalloc_blocks < count) {
        return ERR_NO_SPACE;
    }

    // number of blocks we have allocated so far
    uint32_t alloced = 0;
    // near previous block
    int r = alloc_near_bl(blocks, count, prev_bl, &alloced);
    if (r < 0) { return r; }
    // same block group as previous block
    r = alloc_in_bg(blocks, count, prev_bl / sb->blocks_per_bg, &alloced);
    if (r < 0) { return r; }
    // anywhere
    for (uint32_t bg_num = 0; bg_num < sb->bg_count && alloced < count; bg_num++) {
        r = alloc_in_bg(blocks, count, bg_num, &alloced);
        if (r < 0) { return r; }
    }

    if (alloced != count) {
        return ERR_CORRUPT_STATE;
    }

    // update the superblock
    sb->unalloc_blocks -= count;
    r = sync_superblock(sb);
    return r;
}


int free_block(uint32_t block_num)
{
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


// offset : block number
// count  : block count
int free_blocks_recurs(uint32_t block, uint32_t offset, uint32_t count, uint32_t level)
{
    if (level > 2) {
        panic("free_blocks_recurs : level should be in [0..2]");
    }
    if (level == 0) {
        return (offset == 0 && count > 0) ? free_block(block) : 0;
    }

    uint32_t ch_size = 1;
    for (int i = 0; i < level-1; i++) {
        ch_size *= (sb->block_size / sizeof(uint32_t));
    }
    if (count == 0 || offset >= ch_size * (sb->block_size / sizeof(uint32_t))) {
        return 0;
    }
    // chunk of the first/last block to free
    uint32_t first_ch = offset / ch_size;
    uint32_t last_ch = (offset + count - 1) / ch_size;

    uint32_t* ptrs_block = malloc(sb->block_size);
    int r = read_block(block, 0, sb->block_size, ptrs_block);
    if (r < 0) {
        free(ptrs_block);
        return r;
    }

    for (uint32_t i = first_ch; i <= last_ch && i < (sb->block_size / sizeof(uint32_t)); i++) {
        uint32_t start = i * ch_size;
        r = free_blocks_recurs(
            ptrs_block[i],
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            level-1
        );
        if (r < 0) {
            free(ptrs_block);
            return r;
        }
    }
    free(ptrs_block);
    // here we assume we freed until the end of the file
    if (offset == 0) {
        r = free_block(block);
        return r;
    }
    return 0;
}


// remove (free) some of the inode's blocks
// offset : block number of the first block to remove
// offset+count-1 : block_number of the last block to remove 
int remove_blocks(inode_t* inode, uint32_t offset, uint32_t count)
{
    int r;
    // direct blocks
    for (uint32_t i = offset; i < INODE_DIR_BLOCKS && i < offset + count; i++) {
        r = free_block(inode->dir_blocks[i]);
        if (r < 0) {
            return r;
        }
    }
    // single indirect
    uint32_t start = INODE_DIR_BLOCKS;
    r = free_blocks_recurs(
        inode->single_indir,
        CLIP_OFS(start, offset, count),
        CLIP_CNT(start, offset, count),
        1
    );
    if (r < 0) {
        return r;
    }
    // double indirect
    start += (sb->block_size / sizeof(uint32_t));
    r = free_blocks_recurs(
        inode->double_indir,
        CLIP_OFS(start, offset, count),
        CLIP_CNT(start, offset, count),
        2
    );
    return r;
}


int add_blocks_recurs(inode_t* inode, uint32_t* blocks, uint32_t start, uint32_t count, uint32_t level)
{
    if (level > 2) {
        panic("add_blocks_recurs : level should be in [0..2]\n");
    }
    if (level == 0) {

    }
}


uint32_t min(a, b)
{
    return a < b ? a : b;
}

// offset : block number of the first block to add
// offset+count-1 : block number of the last block to add
int add_blocks(inode_t* inode, uint32_t offset, uint32_t count)
{
    int r;
    // direct blocks
    if (offset < INODE_DIR_BLOCKS) {
        r = alloc_blocks(
            inode->dir_blocks + offset,
            min(INODE_DIR_BLOCKS - offset, count),
            (offset > 0) ? inode->dir_blocks[offset-1] : 0
        );
        if (r < 0) {
            return r;
        }
    }
    // single indirect blocks
    uint32_t start = INODE_DIR_BLOCKS;
    if (offset + count > start) {
        // allocate the ptrs block
        if (offset <= start) {
            r = alloc_blocks(
                &(inode->single_indir),
                1,
                inode->dir_blocks[start-1]
            );
            if (r < 0) {
                return r;
            }
        }
        // read the ptrs block
        uint32_t* ptrs_block = malloc(sb->block_size);
        r = read_block(inode->single_indir, 0, sb->block_size, ptrs_block);
        if (r < 0) {
            free(ptrs_block);
            return 0;
        }
        // allocate the actual blocks
        uint32_t ofs = CLIP_OFS(start, offset, count);
        uint32_t cnt = CLIP_CNT(start, offset, count);
        r = alloc_blocks(
            ptrs_block + ofs,
            cnt,
            (offset <= start ? inode->single_indir : ptrs_block[ofs-1])
        );
        if (r < 0) {
            free(ptrs_block);
            return r;
        }
        // write back the ptrs block
        r = write_block(inode->single_indir, 0, sb->block_size, ptrs_block);
        if (r < 0) {
            free(ptrs_block);
            return r;
        }
        free(ptrs_block);
    }
}


int resize_inode(uint32_t inode_num, uint32_t size)
{
    inode_t* inode = malloc(sizeof(inode_t));
    int r = get_inode(inode_num, inode);
    if (r < 0) {
        free(inode);
        return r;
    }

    uint32_t curr_blocks = div_ceil(inode->fsize, sb->block_size);
    uint32_t new_blocks = div_ceil(size, sb->block_size);
    if (new_blocks < curr_blocks) {
        r = remove_blocks(inode, new_blocks+1, curr_blocks - new_blocks);
        if (r < 0) {
            free(inode);
            return r;
        }
    }
    else if (new_blocks > curr_blocks) {
        r = add_blocks(inode, curr_blocks+1, new_blocks - curr_blocks);
        if (r < 0) {
            free(inode);
            return r;
        }
    }

    inode->fsize = size;
    r = sync_inode(inode_num, inode);
    free(inode);
    return r;
}