// Ext2 inode reading/writing + parsing disk structures into C structs

#include "filesystem/ext2/ext2_internal.h"
#include <stdbool.h>
#include "memory/heap.h"
#include <string.h>
#include "drivers/ata_driver.h"
#include <stdio.h>
#include <panic.h>

int read_block(uint32_t block, uint32_t offset, uint32_t count, void* buf)
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

int write_block(uint32_t block, uint32_t offset, uint32_t count, void* buf)
{
    if (count == 0 || offset >= sb->block_size) {
        return 0;
    }
    if (offset + count > sb->block_size) {
        count = sb->block_size - offset;
    }

    int r;
    // we write part of the block
    if (offset > 0 || count < sb->block_size) {
        void* contents = malloc(sb->block_size);
        // read the contents of the block
        r = read_block(block, 0, sb->block_size, contents);
        if (r < 0) {
            free(contents);
            return r;
        }
        // add the stuff we write
        memcpy(contents+offset, buf, count);
        // write the contents to the block
        uint32_t sect_per_bl = sb->block_size / SECTOR_SIZE;
        uint32_t first_sect = offset / SECTOR_SIZE;
        uint32_t last_sect = (offset+count-1) / SECTOR_SIZE;
        for (uint32_t i = first_sect; i < sect_per_bl && i <= last_sect; i++) {
            r = ata_write_sector(
                block * sect_per_bl + i, 
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
    // we write the whole block
    else {
        // simply write the buffer to the block
        uint32_t sect_per_bl = sb->block_size / SECTOR_SIZE;
        for (uint32_t i = 0; i < sect_per_bl; i++) {
            r = ata_write_sector(
                block * sect_per_bl + i, 
                buf + i * SECTOR_SIZE
            );
            if (r < 0) {
                return ERR_DISK_WRITE;
            }
        }
        return sb->block_size;
    }
}


int write_block_sparse(uint32_t* block, uint32_t offset, uint32_t count, void* buf, uint32_t bg_num)
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
    if (*block == 0) {
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
                break;
            }
        }
        // the block becomes sparse
        if (sparse) {
            free(contents);
            r = free_block(*block);
            *block = 0;
            // don't write the new contents
            return r;
        }
    }
    r = write_block(*block, 0, sb->block_size, contents);   
    free(contents);
    if (r < 0) {
        return r;
    }
    return count;
}

// write the block number bl_num at offset
// to the block 'block' with multiple level of indirection.
// allocates any sparse block on the way to the leaves
// (including 'block')
int write_bl_num_recurs(uint32_t* block, uint32_t offset, uint32_t bl_num, uint32_t bg_num, uint32_t level)
{
    if (level > 2 || level == 0) {
        panic("write_bl_num_recurs : level should be in [1..2]\n");
    }

    int r;
    if (level == 1) {
        r = write_block_sparse(
            block,
            offset * sizeof(uint32_t),
            sizeof(uint32_t),
            &bl_num,
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
        bl_num,
        bg_num,
        level-1
    );
    if (r < 0) {
        free(ptrs_block);
        return r;
    }
    // update the ptrs block if we allocated/deallocated
    // a new block while recursing
    if (ptrs_block[ch] != old_ptr) {
        r = write_block_sparse(
            block, 
            ch * sizeof(uint32_t), 
            sizeof(uint32_t), 
            ptrs_block + ch, 
            bg_num
        );
        if (r < 0) {
            free(ptrs_block);
            return r;
        }
    }
    free(ptrs_block);
    return 0;
}

int read_bl_nums_recurs(uint32_t block, uint32_t offset, uint32_t count, uint32_t* bl_nums, uint32_t level)
{
    if (level > 2 || level == 0) {
        panic("read_bl_nums_recurs : level should be in [1..2]\n");
    }
    if (count == 0) {
        return 0;
    }

    int r;
    if (level == 1) {
        r = read_block(
            block,
            offset * sizeof(uint32_t),
            count * sizeof(uint32_t),
            bl_nums
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
    r = read_block(block, 0, sb->block_size, ptrs_block);
    if (r < 0) {
        free(ptrs_block);
        return r;
    }
    // recurse
    uint32_t first_ch = offset / ch_blocks;
    uint32_t last_ch = (offset + count - 1) / ch_blocks;
    for (uint32_t i = first_ch; i <= last_ch; i++) {
        uint32_t start = i * ch_blocks;
        r = read_bl_nums_recurs(
            ptrs_block[i],
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            bl_nums,
            level-1
        );
        if (r < 0) {
            free(ptrs_block);
            return r;
        }
    }
    free(ptrs_block);
    return 0;
}

int read_bl_nums(uint32_t inode_num, uint32_t offset, uint32_t count, uint32_t* bl_nums)
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
        bl_nums[ofs] = inode->dir_blocks[i];
        ofs++;
    }
    // single indirect blocks
    uint32_t start = INODE_DIR_BLOCKS;
    uint32_t end = start + sb->block_size / sizeof(uint32_t);
    if (offset < end && offset + count > start) {
        r = read_bl_nums_recurs(
            inode->single_indir,
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            bl_nums + ofs, 
            1
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
        r = read_bl_nums_recurs(
            inode->double_indir,
            CLIP_OFS(start, offset, count),
            CLIP_CNT(start, offset, count),
            bl_nums + ofs, 
            2
        );
        if (r < 0) {
            return r;
        }
        ofs += r;
    }
    free(inode);
    return 0;
}

int write_bl_num(uint32_t inode_num, uint32_t offset, uint32_t bl_num)
{
    // get the inode
    int r;
    inode_t* inode = malloc(sizeof(inode_t));
    r = get_inode(inode_num, inode);
    if (r < 0) {
        free(inode);
        return r;
    }

    // direct blocks
    if (offset < INODE_DIR_BLOCKS) {
        inode->dir_blocks[offset] = bl_num;
        goto write_bl_num_success;
    }
    // single indirect blocks
    uint32_t start = INODE_DIR_BLOCKS;
    uint32_t end = start + sb->block_size / sizeof(uint32_t);
    if (start <= offset && offset < end) {
        r = write_bl_num_recurs(
            &(inode->single_indir),
            offset - start,
            bl_num, 
            inode_num / sb->inodes_per_bg,
            1
        );
        if (r < 0) { return r; }
        goto write_bl_num_success; 
    }
    // double indirect blocks
    start = end;
    end += (sb->block_size / sizeof(uint32_t)) * (sb->block_size / sizeof(uint32_t));
    if (start <= offset && offset < end) {
        r = write_bl_num_recurs(
            &(inode->double_indir),
            offset - start,
            bl_num,
            inode_num / sb->inodes_per_bg, 
            2
        );
        if (r < 0) { return r; }
        goto write_bl_num_success;
    }
write_bl_num_failure:
    free(inode);
    // the block designated by 'offset' doesn't exist
    // (it is after the maximum file size)
    return ERR_BLOCK_EXIST;
write_bl_num_success:
    // write back the inode 
    r = sync_inode(inode_num, inode);
    free(inode);
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

    printf("block_nums=");
    for (int i = 0; i < bl_count; i++) {
        printf("%u ", bl_nums[i]);
    }
    printf("\n");

    // actually write
    uint32_t buf_offs = 0;
    for (uint32_t i = 0; i < bl_count; i++) {
        uint32_t start = (i + first_bl) * sb->block_size;
        uint32_t old_bl_num = bl_nums[i];
        r = write_block_sparse(
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
        buf_offs += r;
 
        // the block's sparseness changed
        if (bl_nums[i] != old_bl_num) {
            r = write_bl_num(inode_num, first_bl+i, bl_nums[i]);
            if (r < 0) {
                free(bl_nums);
                return r;
            }
        }
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