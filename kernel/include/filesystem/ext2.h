#pragma once 
#include <stdint.h>

// most functions here return an error code

// the inode doesn't exist
#define ERR_INODE_EXIST (-1)
// the block group doesn't exist
#define ERR_BG_EXIST     (-2)
// the inode has the wrong type
#define ERR_INODE_TYPE  (-3)
// couldn't read disk
#define ERR_DISK_READ   (-4)
// read/write past the end of a file
#define ERR_FILE_BOUNDS (-5)


#define INODE_TYPE_REG   1 // regular file
#define INODE_TYPE_DIR   2 // directory


void init_ext2();

int resize_inode(uint32_t inode, uint32_t size);
// creates an inode for an empty file/directory,
// and without any parent yet 
int alloc_inode(uint32_t* inode, uint32_t type);
// assumes the inode has no parent
// if the inode is a directory, it must be empty.
int free_inode(uint32_t inode);

// file info
int inode_type(uint32_t inode, uint32_t* type);
// for regular files and directories
int inode_fsize(uint32_t inode, uint32_t* size);

// read/write
// the inode can also be a directory
int read_inode(uint32_t inode, uint32_t offset, uint32_t count, void* buf);
int write_inode(uint32_t inode, uint32_t offset, uint32_t count, void* buf);

// max : size of buffer
int dir_list(uint32_t dir, uint32_t* buf, uint32_t max);
int dir_find(uint32_t dir, const char* child_name, uint32_t* child_inode);

// assumes the child already exists
int dir_add_child(uint32_t dir, uint32_t child, const char* name);
// does not delete the child
int dir_rem_child(uint32_t dir, uint32_t child);


