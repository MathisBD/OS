#pragma once 
#include <stdint.h>

// most functions here return an error code

// the inode doesn't exist
#define ERR_INODE_EXIST (-1)
// the inode has the wrong type
#define ERR_INODE_TYPE  (-2)
// etc... (TODO)

// file allocation/resizing
int resize_inode(uint32_t inode, uint32_t size);
int alloc_inode(uint32_t* inode, uint32_t type);
// if the inode is a directory, it must be empty.
int free_inode(uint32_t inode);

// file info
int inode_type(uint32_t inode, uint32_t* type);
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


