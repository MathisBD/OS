#include "filesystem/ext2.h"
#include <stdbool.h>
#include "memory/heap.h"
#include <string.h>
#include "drivers/ata_driver.h"
#include <stdio.h>
#include <panic.h>


// inodes are numbered starting at 1
// blocks and block groups are numbered starting at 0

// we limit ourselves to 32bit disk (and file) sizes,
// to avoid the hassle of converting 32-64bits all the time


#define SECTOR_SIZE         512
// size/layout on disk
#define SB_SIZE             1024
#define SB_OFS              1024
#define INODE_SIZE          128
#define BG_DESCR_SIZE       32

// direct blocks
#define INODE_DIR_BLOCKS 12

#define CLIP_OFS(start, ofs, cnt) \
    (((start) < (ofs)) ? \
    ((ofs) - (start)) : \
    0)

#define CLIP_CNT(start, ofs, cnt) \
    (((start) < (ofs)) ? \
    (cnt) : \
    ((((ofs) + (cnt)) > (start)) ? (((ofs) + (cnt)) - (start)) : 0))

// LOCAL to this file (not visible outside, even by the fs)
// does not reflect physical layout
typedef struct
{
    uint32_t fsize; // size of the file in bytes
    uint32_t type;
    
    // contain block numbers
    uint32_t dir_blocks[INODE_DIR_BLOCKS];
    uint32_t single_indir; // single indirect block
    uint32_t double_indir; // double indirect block
    // no triple indirect block for us
} inode_t;

// does not reflect physical layout
typedef struct 
{
    // size in bytes of each block
    uint32_t block_size;
    
    uint32_t bg_count; // block group count
    uint32_t block_count;
    uint32_t inode_count;

    uint32_t blocks_per_bg;
    uint32_t inodes_per_bg;

    uint32_t unalloc_blocks; 
    uint32_t unalloc_inodes;

    uint32_t root; // root inode
} superblock_t;

// block group descriptor
// does not reflect physical layout
typedef struct 
{
    // these are block numbers
    uint32_t block_bitmap;
    uint32_t inode_bitmap;
    uint32_t inode_table;

    // these are block counts
    uint32_t unalloc_blocks;
    uint32_t unalloc_inodes;
} bg_descr_t;

// maps inode_idx -> inode_t
// all inodes in memory are either local to some function 
// or live in the cache.
// not all inodes are in memory obviously (not enough memory...)
typedef struct {
} inode_cache_t;


// THE superblock
superblock_t* sb;

// offset : offset in the block (0 = start of block)
// it is allowed that offset + count > block_size and/or offset >= block_size
// returns the number of bytes read/written (>= 0) or an error (< 0)
int rw_block(uint32_t block, uint32_t offset, uint32_t count, uint8_t* buf, bool write);

// reads data from disk
int get_superblock(superblock_t* sb);
int get_bg_descr(uint32_t bg_num, bg_descr_t* bg);
int get_inode(uint32_t inode_num, inode_t* inode);
// writes data to disk
int sync_superblock(superblock_t* sb);
int sync_bg_descr(uint32_t bg_num, bg_descr_t* bg);
int sync_inode(uint32_t inode_num, inode_t* inode);


void init_ext2()
{
    sb = malloc(sizeof(superblock_t));
    if (get_superblock(sb) < 0) {
        return -1;
    }
}
  

int rw_block(uint32_t block, uint32_t offset, uint32_t count, uint8_t* buf, bool write)
{
    if (count == 0 || offset >= sb->block_size) {
        return 0;
    }
    if (offset + count >= sb->block_size) {
        count = sb->block_size - offset;
    }

    // special case : sparse block 
    if (block == 0) {
        // WRITE
        if (write) {
            panic("rw_block : can't write to sparse block (not implemented yet)\n");
        }
        // READ
        else {
            memset(buf, 0, count);
        }
    }
    else {
        int r;
        // WRITE
        if (write) {
            r = ata_write(block * sb->block_size + offset, count, buf);
            if (r < 0) {
                return r;
            }
        }
        // READ
        else {
            r = ata_read(block * sb->block_size + offset, count, buf);
            if (r < 0) {
                return r;
            }
        }
    }
    return count;
}

// recursively read/write blocks with multiple levels of indirection
// returns the number of bytes written to/from buf (>= 0) or an error (< 0)
// assumes 0 <= level <= 2
int rw_block_recurs(uint32_t block, uint32_t offset, uint32_t count, uint32_t level, void* buf, bool write)
{
    if (level > 2) {
        panic("read_multi_blocks : level should be in range [0..2]\n");
    }

    if (level == 0) {
        return rw_block(block, offset, count, buf, write);
    }

    // chunk : set of blocks each pointer in the block represents
    uint32_t ch_size = sb->block_size;
    // use level-1 not level here
    for (int i = 0; i < level-1; i++) {
        ch_size *= sb->block_size / sizeof(uint32_t);
    }
    if (count == 0  || offset >= ch_size * (sb->block_size / sizeof(uint32_t))) {
        return 0;
    }

    // chunk of the first/last byte to read
    uint32_t first_ch = offset / ch_size;
    uint32_t last_ch = (offset + count - 1) / ch_size;

    uint32_t* ptrs_block = malloc(sb->block_size);
    int r = rw_block(block, 0, sb->block_size, ptrs_block, false);
    if (r < 0) {
        free(ptrs_block);
        return r;
    }
    
    uint32_t buf_offs = 0;
    for (uint32_t i = first_ch; i <= last_ch; i++) {
        uint32_t start = i * ch_size;
        r = rw_block_recurs(
            ptrs_block[i],
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            level-1,
            buf + buf_offs,
            write
        );
        if (r < 0) {
            free(ptrs_block);
            return r;
        }
        buf_offs += r;
    }

    free(ptrs_block);
    return 0;
}

int get_superblock(superblock_t* sb)
{
    // CAREFUL : the superblock isn't a full 'block' : it is part of 
    // block 0 (or 1 if block_size = 1024)
    void* buf = malloc(SB_SIZE);
    if (ata_read(SB_OFS, SB_SIZE, buf) < 0) {
        free(buf);
        return ERR_DISK_READ;
    }

    uint32_t log_bs = 10 + *((uint32_t*)(buf + 24));
    sb->block_size = 1 << log_bs;

    sb->block_count = *((uint32_t*)(buf + 4));
    sb->inode_count = *((uint32_t*)(buf + 0));
    sb->blocks_per_bg = *((uint32_t*)(buf + 32));
    sb->inodes_per_bg = *((uint32_t*)(buf + 40));
    
    sb->unalloc_blocks = *((uint32_t*)(buf + 12));
    sb->unalloc_inodes = *((uint32_t*)(buf + 16));
    
    // round up
    uint32_t quot = sb->block_count / sb->blocks_per_bg;
    uint32_t rem = sb->block_count % sb->blocks_per_bg;
    if (rem == 0) {
        sb->bg_count = quot;
    }
    else {
        sb->bg_count = quot + 1;
    }
    // the root is always inode 2
    sb->root = 2;

    free(buf);
    return 0;
}

int get_bg_descr(uint32_t bg_num, bg_descr_t* bg)
{
    if (bg_num >= sb->bg_count) {
        return ERR_BG_EXIST;
    }

    // get the bg table block number
    // the bg table is always in the block following the superblock
    uint32_t bg_tab_block;
    if (sb->block_size == 1024) {
        bg_tab_block = 2;
    }
    else {
        bg_tab_block = 1;
    }

    uint32_t block = bg_tab_block + (bg_num * BG_DESCR_SIZE) / sb->block_size;
    uint32_t ofs = (bg_num * BG_DESCR_SIZE) % sb->block_size;
    void* buf = malloc(BG_DESCR_SIZE);
    if (rw_block(block, ofs, BG_DESCR_SIZE, buf, false) < 0) {
        free(buf);
        return ERR_DISK_READ;
    }

    bg->block_bitmap = *((uint32_t*)(buf + 0));
    bg->inode_bitmap = *((uint32_t*)(buf + 4));
    bg->inode_table = *((uint32_t*)(buf + 8));

    bg->unalloc_blocks = *((uint16_t*)(buf + 12));
    bg->unalloc_inodes = *((uint16_t*)(buf + 14));

    free(buf);
    return 0;
}

int get_inode(uint32_t inode_num, inode_t* inode)
{
    if (inode_num > sb->inode_count) {
        return ERR_INODE_EXIST;
    }

    uint32_t inode_idx = (inode_num - 1) % sb->inodes_per_bg;
    uint32_t bg_num = (inode_num - 1) / sb->inodes_per_bg;

    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    int r = get_bg_descr(bg_num, bg);
    if (r < 0) {
        free(bg);
        return r;
    }

    // index the inode table
    uint32_t ofs = (inode_idx * INODE_SIZE) % sb->block_size;
    uint32_t block = bg->inode_table + (inode_idx * INODE_SIZE) / sb->block_size;
    void* buf = malloc(INODE_SIZE);
    r = rw_block(block, ofs, INODE_SIZE, buf, false);
    if (r < 0) {
        free(buf);
        return r;
    }

    // only handle 32-bit file sizes for now
    inode->fsize = *((uint32_t*)(buf + 4));
    uint16_t type = *((uint16_t*)(buf + 0)) & 0xF000;
    switch (type) {
    case 0x4000: inode->type = INODE_TYPE_DIR; break;
    case 0x8000: inode->type = INODE_TYPE_REG; break;
    default: return ERR_INODE_TYPE;
    }

    for (uint32_t i = 0; i < INODE_DIR_BLOCKS; i++) {
        inode->dir_blocks[i] = *((uint32_t*)(buf + 40 + 4*i));
    }
    inode->single_indir = *((uint32_t*)(buf + 88));
    inode->double_indir = *((uint32_t*)(buf + 92));

    free(buf);
    return 0;    
}   

int inode_type(uint32_t inode_num, uint32_t* type)
{
    inode_t* inode = malloc(sizeof(inode_t)); 
    int r = get_inode(inode_num, inode);
    if (r < 0) {
        free(inode);
        return r;
    }
    *type = inode->type;
    free(inode);
    return 0;
}

int inode_fsize(uint32_t inode_num, uint32_t* fsize)
{
    inode_t* inode = malloc(sizeof(inode_t)); 
    int r = get_inode(inode_num, inode);
    if (r < 0) {
        free(inode);
        return r;
    }
    *fsize = inode->fsize;
    free(inode);
    return 0;
}

int rw_inode(uint32_t inode_num, uint32_t offset, uint32_t count, void* buf, bool write)
{
    inode_t* inode = malloc(sizeof(inode_t));
    int r = get_inode(inode_num, inode);
    if (r < 0) {
        free(inode);
        return r;
    }

    if (offset + count >= inode->fsize) {
        free(inode);
        return ERR_FILE_BOUNDS;
    }

    uint32_t buf_offs = 0;
    
    // direct blocks
    for (uint32_t i = 0; i < INODE_DIR_BLOCKS; i++) {
        uint32_t start = i * sb->block_size;
        r = rw_block(
            inode->dir_blocks[i], 
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count), 
            buf + buf_offs,
            write
        );
        if (r < 0) {
            free(inode);
            return r;
        }
        buf_offs += r;
    }

    // single indirect block
    uint32_t start = INODE_DIR_BLOCKS * sb->block_size; 
    r = rw_block_recurs(
        inode->single_indir,  
        CLIP_OFS(start, offset, count),
        CLIP_CNT(start, offset, count),
        1, 
        buf + buf_offs,
        write
    ); 
    if (r < 0) {
        free(inode);
        return r;
    }
    buf_offs += r;
    // double indirect block
    start += sb->block_size * (sb->block_size / sizeof(uint32_t));
    r = rw_block_recurs(
        inode->double_indir,  
        CLIP_OFS(start, offset, count),
        CLIP_CNT(start, offset, count),
        2, 
        buf + buf_offs,
        write
    ); 
    if (r < 0) {
        free(inode);
        return r;
    }
    buf_offs += r;

    free(inode);
    return 0;
}

int read_inode(uint32_t inode_num, uint32_t offset, uint32_t count, void* buf)
{
    return rw_inode(inode_num, offset, count, buf, false);
}

int write_inode(uint32_t inode_num, uint32_t offset, uint32_t count, void* buf)
{
    return rw_inode(inode_num, offset, count, buf, true);
}