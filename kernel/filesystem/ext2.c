#include "filesystem/ext2.h"
#include <stdbool.h>

#define INODE_DIR_BLOCKS 12

#define INODE_TYPE_REG   1 // regular file
#define INODE_TYPE_DIR   2 // directory

// LOCAL to this file (not visible outside, even by the fs)
// does not reflect physical layout
typedef struct
{
    uint32_t fsize;
    uint32_t type;
    // contains block numbers
    uint32_t dir_blocks[INODE_DIR_BLOCKS];
    uint32_t single_indir;
    uint32_t double_indir;
    uint32_t triple_indir;
} inode_t;

// does not reflect physical layout
typedef struct 
{
    uint32_t block_size;
    
    uint32_t bg_count;
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


// reads data from disk
int get_superblock(superblock_t* sb);
int get_bg_descr(uint32_t bg_num, bg_descr_t* bg);
int get_inode(uint32_t inode_num, inode_t* inode);
// writes data to disk 
int sync_superblock(superblock_t* sb);
int sync_bg_descr(uint32_t bg_num, bg_descr_t* bg);
int sync_inode(uint32_t inode_num, inode_t* inode);

// creates an inode for an empty file/directory,
// and without any parent yet 
int alloc_inode(uint32_t* inode, uint32_t type);
// assumes the inode has no parent,
// and if it is a directory, that it is empty
int free_inode(uint32_t inode);

