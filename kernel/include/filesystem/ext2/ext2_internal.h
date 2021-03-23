#pragma once 
#include <stdint.h>


// IMPORTANT : include the public interface so that
// different source files can access the public functions defined
// in each other
#include "filesystem/ext2/ext2.h"


// inodes are numbered starting at 1
// blocks and block groups are numbered starting at 0

// we limit ourselves to 32bit disk (and file) sizes,
// to avoid the hassle of converting 32-64bits all the time

// size/layout on disk
#define SECTOR_SIZE         512
#define SB_SIZE             1024
#define SB_OFS              1024
#define INODE_SIZE          128
#define BG_DESCR_SIZE       32

// direct blocks
#define INODE_DIR_BLOCKS 12

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
    // bit 0 of the inode bitmap corresponds to inode 1
    // entry 0 of the inode table corresponds to inode 1
    uint32_t block_bitmap;
    uint32_t inode_bitmap; 
    uint32_t inode_table;

    // these are block counts
    uint32_t unalloc_blocks;
    uint32_t unalloc_inodes;
} bg_descr_t;


#define CLIP_OFS(start, ofs, cnt) \
    (((start) < (ofs)) ? \
    ((ofs) - (start)) : \
    0)

#define CLIP_CNT(start, ofs, cnt) \
    (((start) < (ofs)) ? \
    (cnt) : \
    ((((ofs) + (cnt)) > (start)) ? (((ofs) + (cnt)) - (start)) : 0))


// THE superblock (defined in ext2_gen.c)
extern superblock_t* sb;

// reads data from disk
int get_superblock(superblock_t* sb);
int get_bg_descr(uint32_t bg_num, bg_descr_t* bg);
int get_inode(uint32_t inode_num, inode_t* inode);

// writes data to disk
int sync_superblock(superblock_t* sb);
int sync_bg_descr(uint32_t bg_num, bg_descr_t* bg);
int sync_inode(uint32_t inode_num, inode_t* inode);

// doesn't resize the inode, simply
// changes the fsize field
int set_inode_fsize(uint32_t inode_num, uint32_t fsize);

// offset : offset in the block (0 = start of block)
// it is allowed that offset + count > block_size and/or offset >= block_size
// returns the number of bytes read/written from buf (>= 0) or an error (< 0)
int read_block(uint32_t block, uint32_t offset, uint32_t count, void* buf);
int write_block(uint32_t block, uint32_t offset, uint32_t count, void* buf);
// this should only be used for writting to an
// inode's data block (or indirect blocks), as it can
// deallocate the block if it becomes sparse (we of course
// don't want that for e.g. a block in an inode table)
// if after the write the block would contain only zeros,
// frees the block and sets *block=0 
// if *block==0 and after the write the block would not be sparse,
// allocates a block (in block group bg_num) and sets *block to its number
int write_block_sparse(uint32_t* block, uint32_t offset, uint32_t count, void* buf, uint32_t bg_num);

// example : read_bl_nums(42, 2, 3, buf) reads the blocks numbers
// of blocks 2, 3, 4 of inode 42 and writes them into buf
int read_bl_nums(uint32_t inode_num, uint32_t offset, uint32_t count, uint32_t* bl_nums);
int write_bl_num(uint32_t inode_num, uint32_t offset, uint32_t bl_num);

// bg_num : prefered block group. 
// if no more space in this bg, allocate in any bg.
// alloc_block should only be used when trying to write
// to a sparse block
int alloc_block(uint32_t* block, uint32_t bg_num);
int free_block(uint32_t block);
