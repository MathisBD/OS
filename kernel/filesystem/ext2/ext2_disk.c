#include "filesystem/ext2/ext2_internal.h"
#include "memory/kheap.h"
#include <stdbool.h>
#include "drivers/ata_driver.h"

// size of buf : SB_SIZE
int rw_raw_superblock(void* buf, bool write)
{
    // CAREFUL : the superblock isn't a full 'block' : it is part of 
    // block 0 (or 1 if block_size = 1024).
    // we can't use read_block here since it relies on the superblock being initialized
    
    if (write) {
        if (ata_write_sector(SB_OFS / SECTOR_SIZE, buf) < 0) {
            return EXT2_ERR_DISK_WRITE;
        }
        if (ata_write_sector(SB_OFS / SECTOR_SIZE + 1, buf + SECTOR_SIZE) < 0) {
            return EXT2_ERR_DISK_WRITE;
        }
    }
    else {
        if (ata_read(SB_OFS, SB_SIZE, buf) < 0) {
            return EXT2_ERR_DISK_READ;
        }
    }
    return 0;
}

// size of buf : BG_DESCR_SIZE
int rw_raw_bg_desr(uint32_t bg_num, void* buf, bool write)
{
    if (bg_num >= sb->bg_count) {
        return EXT2_ERR_BG_EXIST;
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
    if (write) {
        int r = write_block(block, ofs, BG_DESCR_SIZE, buf);
        return r < 0 ? r : 0;
    }
    else {
        int r = read_block(block, ofs, BG_DESCR_SIZE, buf);
        return r < 0 ? r : 0;
    }
}

// size of buf : INODE_SIZE
int rw_raw_inode(uint32_t inode_num, void* buf, bool write)
{
    // remember inodes start at 1 not 0
    if (inode_num > sb->inode_count) {
        return EXT2_ERR_INODE_EXIST;
    }

    uint32_t inode_idx = (inode_num - 1) % sb->inodes_per_bg;
    uint32_t bg_num = (inode_num - 1) / sb->inodes_per_bg;

    // get the corresponding block group descriptor
    bg_descr_t* bg = kmalloc(sizeof(bg_descr_t));
    int r = get_bg_descr(bg_num, bg);
    if (r < 0) {
        kfree(bg);
        return r;
    }

    // index the inode table
    uint32_t ofs = (inode_idx * INODE_SIZE) % sb->block_size;
    uint32_t block = bg->inode_table + (inode_idx * INODE_SIZE) / sb->block_size;
    kfree(bg);

    if (write) {
        int r = write_block(block, ofs, INODE_SIZE, buf);
        return r < 0 ? r : 0;
    }
    else {
        int r = read_block(block, ofs, INODE_SIZE, buf);
        return r < 0 ? r : 0;
    }
}

int get_superblock(superblock_t* sb)
{
    void* buf = kmalloc(SB_SIZE);
    int r = rw_raw_superblock(buf, false);
    if (r < 0) {
        kfree(buf);
        return r;
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
    kfree(buf);

    // sanity checks
    if (sb->blocks_per_bg != 8 * sb->block_size) {
        return EXT2_ERR_CORRUPT_STATE;
    }
    return 0;
}

int sync_superblock(superblock_t* sb)
{
    void* buf = kmalloc(SB_SIZE);
    int r = rw_raw_superblock(buf, false);
    if (r < 0) {
        kfree(buf);
        return r;
    }

    // only write the data that makes sense to modify
    *((uint32_t*)(buf + 12)) = sb->unalloc_inodes;
    *((uint32_t*)(buf + 16)) = sb->unalloc_blocks;

    r = rw_raw_superblock(buf, true);
    kfree(buf);
    return r;
}

int get_bg_descr(uint32_t bg_num, bg_descr_t* bg)
{
    void* buf = kmalloc(BG_DESCR_SIZE);
    int r = rw_raw_bg_desr(bg_num, buf, false);
    if (r < 0) {
        kfree(buf);
        return r;
    }

    bg->block_bitmap = *((uint32_t*)(buf + 0));
    bg->inode_bitmap = *((uint32_t*)(buf + 4));
    bg->inode_table = *((uint32_t*)(buf + 8));

    bg->unalloc_blocks = *((uint16_t*)(buf + 12));
    bg->unalloc_inodes = *((uint16_t*)(buf + 14));

    kfree(buf);
    return 0;
}

int sync_bg_descr(uint32_t bg_num, bg_descr_t* bg)
{
    void* buf = kmalloc(BG_DESCR_SIZE);
    int r = rw_raw_bg_desr(bg_num, buf, false);
    if (r < 0) {
        kfree(buf);
        return r;
    }

    // only write the data that makes sense to modify
    *((uint16_t*)(buf + 12)) = bg->unalloc_blocks;
    *((uint16_t*)(buf + 14)) = bg->unalloc_inodes;

    r = rw_raw_bg_desr(bg_num, buf, true);
    kfree(buf);
    return r;
}

int get_inode(uint32_t inode_num, inode_t* inode)
{
    void* buf = kmalloc(INODE_SIZE);
    int r = rw_raw_inode(inode_num, buf, false);
    if (r < 0) {
        kfree(buf);
        return r;
    }
    
    inode->fsize = *((uint32_t*)(buf + 4));
    uint16_t type = *((uint16_t*)(buf + 0)) & 0xF000;
    switch (type) {
    case 0x4000: inode->type = EXT2_INODE_TYPE_DIR; break;
    case 0x8000: inode->type = EXT2_INODE_TYPE_REG; break;
    default: return EXT2_ERR_INODE_TYPE;
    }

    for (uint32_t i = 0; i < INODE_DIR_BLOCKS; i++) {
        inode->dir_blocks[i] = *((uint32_t*)(buf + 40 + 4*i));
    }
    inode->single_indir = *((uint32_t*)(buf + 88));
    inode->double_indir = *((uint32_t*)(buf + 92));

    kfree(buf);
    return 0;    
}   

int sync_inode(uint32_t inode_num, inode_t* inode)
{
    void* buf = kmalloc(INODE_SIZE);
    int r = rw_raw_inode(inode_num, buf, false);
    if (r < 0) {
        kfree(buf);
        return r;
    }
    
    // only write the data that makes sense to modify
    *((uint32_t*)(buf + 4)) = inode->fsize;
    uint16_t type;
    switch(inode->type) {
    case EXT2_INODE_TYPE_DIR: type = 0x4000; break;
    case EXT2_INODE_TYPE_REG: type = 0x8000; break;
    default: return EXT2_ERR_INODE_TYPE;
    }
    // AND with 0x0FFF to erase the old type, OR to add the new type
    *((uint16_t*)(buf + 0)) = type | (0x0FFF & *((uint16_t*)(buf + 0)));

    for (uint32_t i = 0; i < INODE_DIR_BLOCKS; i++) {    
        *((uint32_t*)(buf + 40 + 4*i)) = inode->dir_blocks[i];
    }
    *((uint32_t*)(buf + 88)) = inode->single_indir;
    *((uint32_t*)(buf + 92)) = inode->double_indir;

    r = rw_raw_inode(inode_num, buf, true);
    kfree(buf);
    return r;
}
