// Ext2 initialisation and generalities.

#include "filesystem/ext2/ext2_internal.h"
#include "memory/kheap.h"
#include <stdio.h>
#include <string.h>
#include <panic.h>


// We are the file that has to declare this variable.
superblock_t* sb;


int init_ext2()
{
    sb = kmalloc(sizeof(superblock_t));
    int r = get_superblock(sb);
    return r;
}
  

int get_inode_type(uint32_t inode_num, uint32_t* type)
{
    inode_t* inode = kmalloc(sizeof(inode_t)); 
    int r = get_inode(inode_num, inode);
    if (r < 0) {
        kfree(inode);
        return r;
    }
    *type = inode->type;
    kfree(inode);
    return 0;
}


int get_inode_fsize(uint32_t inode_num, uint32_t* fsize)
{
    inode_t* inode = kmalloc(sizeof(inode_t)); 
    int r = get_inode(inode_num, inode);
    if (r < 0) {
        kfree(inode);
        return r;
    }
    *fsize = inode->fsize;
    kfree(inode);
    return 0;
}

int set_inode_fsize(uint32_t inode_num, uint32_t fsize)
{
    inode_t* inode = kmalloc(sizeof(inode_t)); 
    int r = get_inode(inode_num, inode);
    if (r < 0) {
        kfree(inode);
        return r;
    }
    inode->fsize = fsize;

    r = sync_inode(inode_num, inode);
    kfree(inode);
    return r;
}
