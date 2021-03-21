// Ext2 inode reading/writing + parsing disk structures into C structs

#include "filesystem/ext2/ext2_internal.h"
#include <stdbool.h>
#include "memory/heap.h"
#include <string.h>
#include "drivers/ata_driver.h"
#include <stdio.h>
#include <panic.h>

int read_block(uint32_t block, uint32_t offset, uint32_t count, uint8_t* buf)
{
    if (count == 0 || offset >= sb->block_size) {
        return 0;
    }
    if (offset + count > sb->block_size) {
        count = sb->block_size - offset;
    }

    // special case : sparse block 
    if (block == 0) {
        memset(buf, 0, count);
    }
    else {
        int r = ata_read(block * sb->block_size + offset, count, buf);
        if (r < 0) {
            return ERR_DISK_READ;
        }
    }
    return count;
}

int write_block(uint32_t* block, uint32_t offset, uint32_t count, uint8_t* buf, uint32_t bg_num)
{
    if (count == 0 || offset >= sb->block_size) {
        return 0;
    }
    if (offset + count > sb->block_size) {
        count = sb->block_size - offset;
    }

    int r;
    // contains block_size bytes : the new contents of the block
    void* contents;
    // special case : sparse block 
    if (block == 0) {
        // check if the block stays sparse
        bool sparse = true;
        for (uint32_t i = 0; i < count; i++) {
            if (*((uint8_t*)(buf + i)) != 0) {
                sparse = false;
            }
        }
        // stays sparse : nothing to do
        if (sparse) {
            return 0;
        }
        // allocate the block
        r = alloc_block(block, bg_num);
        if (r < 0) {
            return r;
        }
        // create the new contents
        contents = malloc(sb->block_size);
        memset(contents, 0, sb->block_size);
        memcpy(contents + offset, buf, count);
    }
    // non sparse block
    else {
        // read the contents of the block
        void* contents = malloc(sb->block_size);
        r = read_block(*block, 0, sb->block_size, contents);
        if (r < 0) {
            free(contents);
            return r;
        }
        // add the stuff we write
        memcpy(contents+offset, buf, count);
        // check if the block becomes sparse
        bool sparse = true;
        for (uint32_t i = 0; i < sb->block_size; i++) {
            if (*((uint8_t*)(contents + i)) != 0) {
                sparse = false;
            }
        }
        // the block becomes sparse
        if (sparse) {
            r = free_block(*block);
            if (r < 0) {
                free(contents);
                return r;
            }
            *block = 0;
            // don't write the new contents
            return 0;
        }
    }
    // write the contents to the block
    uint32_t sect_per_bl = sb->block_size / SECTOR_SIZE;
    uint32_t first_sect = offset / SECTOR_SIZE;
    uint32_t last_sect = (offset+count-1) / SECTOR_SIZE;
    for (uint32_t i = first_sect; i < sect_per_bl && i <= last_sect; i++) {
        int r = ata_write_sector(
            *block * sect_per_bl + i, 
            contents + i * SECTOR_SIZE
        );
        if (r < 0) {
            free(contents);
            return ERR_DISK_WRITE;
        }
    }
    free(contents);
    return count;
}

// write the block number bl_num at offset
// to the block 'block' with multiple level of indirection.
// allocates any sparse block on the way to the leaves
// (including 'block')
int write_bl_num_recurs(uint32_t* block, uint32_t offset, uint32_t bl_num, uint32_t level, uint32_t bg_num)
{
    if (level > 2 || level == 0) {
        panic("rw_bl_nums_recurs : level should be in [1..2]\n");
    }

    int r;
    if (level == 0) {
        r = write_block(
            block,
            offset * sizeof(uint32_t),
            sizeof(uint32_t),
            bl_num,
            bg_num
        );
        return r;
    }

    // chunk : set of blocks each pointer in the block represents
    uint32_t ch_blocks = 1;
    // use level-1 not level here
    for (int i = 0; i < level-1; i++) {
        ch_blocks *= sb->block_size / sizeof(uint32_t);
    }
    if (offset >= ch_blocks * (sb->block_size / sizeof(uint32_t))) {
        return 0;
    }
    // read the ptrs block
    uint32_t* ptrs_block = malloc(sb->block_size);
    r = read_block(*block, 0, sb->block_size, ptrs_block);
    if (r < 0) {
        free(ptrs_block);
        return r;
    }
    // recurse
    uint32_t ch = offset / ch_blocks;
    uint32_t start = ch * ch_blocks;
    uint32_t old_ptr = ptrs_block[ch];
    r = write_bl_num_recurs(
        ptrs_block + ch,
        CLIP_OFS(start, offset, 1),
        level-1,
        bl_num,
        bg_num
    );
    if (r < 0) {
        free(ptrs_block);
        return r;
    }
    // update the ptrs block if we allocated
    // a new block while recursing
    if (ptrs_block[ch] != old_ptr) {
        r = write_block(block, ch, 1, ptrs_block + ch, bg_num);
        if (r < 0) {
            free(ptrs_block);
            return r;
        }
    }
    free(ptrs_block);
    return 0;
}

int rw_bl_nums(uint32_t inode_num, uint32_t offset, uint32_t count, uint32_t* bl_nums, bool write)
{
    // get the inode
    int r;
    inode_t* inode = malloc(sizeof(inode_t));
    r = get_inode(inode_num, inode);
    if (r < 0) {
        free(inode);
        return r;
    }

    uint32_t ofs = 0;
    // direct blocks
    for (uint32_t i = offset; i < INODE_DIR_BLOCKS && i < offset + count; i++) {
        if (write) {
            inode->dir_blocks[i] = bl_nums[ofs];
        }
        else {
            bl_nums[ofs] = inode->dir_blocks[i];
        }
        ofs++;
    }
    // single indirect blocks
    uint32_t start = INODE_DIR_BLOCKS;
    uint32_t end = start + sb->block_size / sizeof(uint32_t);
    if (offset < end && offset + count > start) {
        r = force_sparse_block(&(inode->single_indir), inode_num / sb->inodes_per_bg);
        if (r < 0) {
            return r;
        }
        r = rw_bl_nums_recurs(
            inode->single_indir,
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            bl_nums + ofs, 
            1,
            write
        );
        if (r < 0) {
            return r;
        }
        ofs += r;
    }
    // double indirect blocks
    start = end;
    end += (sb->block_size / sizeof(uint32_t)) * (sb->block_size / sizeof(uint32_t));
    if (offset < end && offset + count > start) {
        r = force_sparse_block(&(inode->double_indir), inode_num / sb->inodes_per_bg);
        if (r < 0) {
            return r;
        }
        r = rw_bl_nums_recurs(
            inode->double_indir,
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            bl_nums + ofs, 
            2,
            write
        );
        if (r < 0) {
            return r;
        }
        ofs += r;
    }
    // write back and free the inode
    if (write) {
        r = sync_inode(inode_num, inode);
    }
    free(inode);
    if (r < 0) {
        return r;
    }
    return ofs;
}

int write_bl_nums(uint32_t inode_num, uint32_t offset, uint32_t count, uint32_t* bl_nums)
{
    return rw_bl_nums(inode_num, offset, count, bl_nums, true);
}

int read_bl_nums(uint32_t inode_num, uint32_t offset, uint32_t count, uint32_t* bl_nums)
{
    return rw_bl_nums(inode_num, offset, count, bl_nums, false);
}

// size of buf : SB_SIZE
int rw_raw_superblock(void* buf, bool write)
{
    // CAREFUL : the superblock isn't a full 'block' : it is part of 
    // block 0 (or 1 if block_size = 1024)
    // we can't use read_block here since it relies on the superblock being initialized
    
    if (write) {
        if (ata_write(SB_OFS, SB_SIZE, buf) < 0) {
            return ERR_DISK_WRITE;
        }
    }
    else {
        if (ata_read(SB_OFS, SB_SIZE, buf) < 0) {
            return ERR_DISK_READ;
        }
    }
    return 0;
}

// size of buf : BG_DESCR_SIZE
int rw_raw_bg_desr(uint32_t bg_num, void* buf, bool write)
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
    int r = rw_block(block, ofs, BG_DESCR_SIZE, buf, write);
    return r;
}

// size of buf : INODE_SIZE
int rw_raw_inode(uint32_t inode_num, void* buf, bool write)
{
    // remember inodes start at 1 not 0
    if (inode_num > sb->inode_count) {
        return ERR_INODE_EXIST;
    }

    uint32_t inode_idx = (inode_num - 1) % sb->inodes_per_bg;
    uint32_t bg_num = (inode_num - 1) / sb->inodes_per_bg;

    // get the corresponding block group descriptor
    bg_descr_t* bg = malloc(sizeof(bg_descr_t));
    int r = get_bg_descr(bg_num, bg);
    if (r < 0) {
        free(bg);
        return r;
    }

    // index the inode table
    uint32_t ofs = (inode_idx * INODE_SIZE) % sb->block_size;
    uint32_t block = bg->inode_table + (inode_idx * INODE_SIZE) / sb->block_size;
    r = rw_block(block, ofs, INODE_SIZE, buf, write);
    free(bg);
    return r;
}

int get_superblock(superblock_t* sb)
{
    void* buf = malloc(SB_SIZE);
    int r = rw_raw_superblock(buf, false);
    if (r < 0) {
        free(buf);
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
    free(buf);

    // sanity checks
    if (sb->blocks_per_bg != 8 * sb->block_size) {
        return ERR_CORRUPT_STATE;
    }
    return 0;
}

int sync_superblock(superblock_t* sb)
{
    void* buf = malloc(SB_SIZE);
    int r = rw_raw_superblock(buf, false);
    if (r < 0) {
        free(buf);
        return r;
    }

    // only write the data that makes sense to modify
    *((uint32_t*)(buf + 12)) = sb->unalloc_inodes;
    *((uint32_t*)(buf + 16)) = sb->unalloc_blocks;

    r = rw_raw_superblock(buf, true);
    free(buf);
    return r;
}

int get_bg_descr(uint32_t bg_num, bg_descr_t* bg)
{
    void* buf = malloc(BG_DESCR_SIZE);
    int r = rw_raw_bg_desr(bg_num, buf, false);
    if (r < 0) {
        free(buf);
        return r;
    }

    bg->block_bitmap = *((uint32_t*)(buf + 0));
    bg->inode_bitmap = *((uint32_t*)(buf + 4));
    bg->inode_table = *((uint32_t*)(buf + 8));

    bg->unalloc_blocks = *((uint16_t*)(buf + 12));
    bg->unalloc_inodes = *((uint16_t*)(buf + 14));

    free(buf);
    return 0;
}

int sync_bg_descr(uint32_t bg_num, bg_descr_t* bg)
{
    void* buf = malloc(BG_DESCR_SIZE);
    int r = rw_raw_bg_desr(bg_num, buf, false);
    if (r < 0) {
        free(buf);
        return r;
    }

    // only write the data that makes sense to modify
    *((uint16_t*)(buf + 12)) = bg->unalloc_blocks;
    *((uint16_t*)(buf + 14)) = bg->unalloc_inodes;

    r = rw_raw_bg_desr(bg_num, buf, true);
    free(buf);
    return r;
}

int get_inode(uint32_t inode_num, inode_t* inode)
{
    void* buf = malloc(INODE_SIZE);
    int r = rw_raw_inode(inode_num, buf, false);
    if (r < 0) {
        free(buf);
        return r;
    }
    
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

int sync_inode(uint32_t inode_num, inode_t* inode)
{
    void* buf = malloc(INODE_SIZE);
    int r = rw_raw_inode(inode_num, buf, false);
    if (r < 0) {
        free(buf);
        return r;
    }
    
    // only write the data that makes sense to modify
    *((uint32_t*)(buf + 4)) = inode->fsize;
    uint16_t type;
    switch(inode->type) {
    case INODE_TYPE_DIR: type = 0x4000; break;
    case INODE_TYPE_REG: type = 0x8000; break;
    default: return ERR_INODE_TYPE;
    }
    // AND with 0x0FFF to erase the old type, OR to add the new type
    *((uint16_t*)(buf + 0)) = type | (0x0FFF & *((uint16_t*)(buf + 0)));

    for (uint32_t i = 0; i < INODE_DIR_BLOCKS; i++) {    
        *((uint32_t*)(buf + 40 + 4*i)) = inode->dir_blocks[i];
    }
    *((uint32_t*)(buf + 88)) = inode->single_indir;
    *((uint32_t*)(buf + 92)) = inode->double_indir;

    r = rw_raw_inode(inode_num, buf, true);
    free(buf);
    return r;
}

int write_inode(uint32_t inode_num, uint32_t offset, uint32_t count, void* buf)
{
    uint32_t first_bl = offset / sb->block_size;
    uint32_t last_bl = (offset + count - 1) / sb->block_size;
    uint32_t bl_count = last_bl - first_bl + 1;
    
    // get the block numbers
    uint32_t* bl_nums = malloc(bl_count * sizeof(uint32_t));
    int r = read_bl_nums(inode_num, first_bl, bl_count, bl_nums);
    if (r < 0) {
        free(bl_nums);
        return r;   
    }
    // actually write
    uint32_t buf_offs = 0;
    for (uint32_t i = 0; i < bl_count; i++) {
        uint32_t start = (i + first_bl) * sb->block_size;
        uint32_t old_bl_num = bl_nums[i];
        r = write_block(
            bl_nums + i,
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            buf + buf_offs,
            inode_num / sb->inodes_per_bg
        );
        if (r < 0) {
            free(bl_nums);
            return r;
        }
        // the block's sparseness changed
        if (bl_nums[i] != old_bl_num) {
            r = write_bl_nums(inode_num, first_bl+i, 1, bl_nums + i);
            if (r < 0) {
                free(bl_nums);
                return r;
            }
        }
        buf_offs += r;
    }
    free(bl_nums);
    return 0;
}


int read_inode(uint32_t inode_num, uint32_t offset, uint32_t count, void* buf)
{
    uint32_t first_bl = offset / sb->block_size;
    uint32_t last_bl = (offset + count - 1) / sb->block_size;
    uint32_t bl_count = last_bl - first_bl + 1;
    
    // get the block numbers
    uint32_t* bl_nums = malloc(bl_count * sizeof(uint32_t));
    int r = read_bl_nums(inode_num, first_bl, bl_count, bl_nums);
    if (r < 0) {
        free(bl_nums);
        return r;   
    }
    // actually read
    uint32_t buf_offs = 0;
    for (uint32_t i = 0; i < bl_count; i++) {
        uint32_t start = (i + first_bl) * sb->block_size;
        r = read_block(
            bl_nums[i],
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            buf + buf_offs
        );
        if (r < 0) {
            free(bl_nums);
            return r;
        }
        buf_offs += r;
    }
    free(bl_nums);
    return 0;
}