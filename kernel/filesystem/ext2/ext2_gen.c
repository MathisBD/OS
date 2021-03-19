// Ext2 initialisation and generalities.

#include "filesystem/ext2/ext2_internal.h"
#include "memory/heap.h"
#include <stdio.h>
#include <string.h>


// We are the file that has to declare this variable.
superblock_t* sb;


void init_ext2()
{
    sb = malloc(sizeof(superblock_t));
    if (get_superblock(sb) < 0) {
        panic("init_ext2 : could not get the superblock\n");
    }
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