#pragma once 
#include <stdint.h>
#include "filesystem/ext2.h"

// This is the layer between the ext2 filesystem and the kernel/user
// Kind of like a VFS except it can only support EXT2
// In particular code in this file should not know any details
// about ext2 implementation

// a handle to an inode
// several 'file' objects can point to the same inode
struct f_handle
{
    uint32_t inode;
    uint32_t cur_pos;
};