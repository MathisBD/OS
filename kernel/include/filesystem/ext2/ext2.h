#pragma once 
#include <stdint.h>


// This is the file to include from outside the ext2 subsystem.
// Other ext2 headers should be private to the ext2 subsystem.
// Ext2 implementations sources should instead include "ext2_internal.h".
// (dependency graph : ext2.h <- ext2_internal.h <- ext2_*.c)
// Most functions here return an error code.


// the inode doesn't exist
#define ERR_INODE_EXIST     (-1)
// the block group doesn't exist
#define ERR_BG_EXIST        (-2)
// the inode has the wrong type
#define ERR_INODE_TYPE      (-3)
// couldn't read disk
#define ERR_DISK_READ       (-4)
// couldn't write disk
#define ERR_DISK_WRITE      (-5)
// read/write past the end of a file
#define ERR_FILE_BOUNDS     (-6)
// inconsistent disk state
// (e.g. unallocated_blocks > 0 but the block bitmap is full)
#define ERR_CORRUPT_STATE   (-7)
// no more disk space
// (i.e. no more unallocated inodes/blocks)
#define ERR_NO_SPACE        (-8)

#define INODE_TYPE_REG   1 // regular file
#define INODE_TYPE_DIR   2 // directory


void init_ext2();

// file info
int inode_type(uint32_t inode, uint32_t* type);
// for regular files and directories
int inode_fsize(uint32_t inode, uint32_t* size);

// read/write
// the inode can also be a directory
int read_inode(uint32_t inode, uint32_t offset, uint32_t count, void* buf);
int write_inode(uint32_t inode, uint32_t offset, uint32_t count, void* buf);

int resize_inode(uint32_t inode, uint32_t size);
// creates an inode for an empty file/directory,
// and without any parent yet
// (finds a free inode and allocates it) 
int alloc_inode(uint32_t* inode, uint32_t type);
// assumes the inode has no parent
// and that it is empty (fsize == 0)
int free_inode(uint32_t inode);

// max : size of buffer
int dir_list(uint32_t dir, uint32_t* buf, uint32_t max);
int dir_find(uint32_t dir, const char* child_name, uint32_t* child_inode);

// assumes the child already exists
int dir_add_child(uint32_t dir, uint32_t child, const char* name);
// does not delete the child
int dir_rem_child(uint32_t dir, uint32_t child);
