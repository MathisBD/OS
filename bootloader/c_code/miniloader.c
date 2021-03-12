#include "miniloader.h"
#include "minielf.h"
#include <stdbool.h>
#include "vga_minidriver.h"

// addresses we can load temporary stuff at
// we (the second stage) are at 0x8000
#define TMP_LOAD_ADDR_1 0x10000
#define TMP_LOAD_ADDR_2 0x20000

#define SECTOR_SIZE 512
#define EXT2_DIRECT_BLOCKS 12


extern uint32_t block_size;
extern uint32_t sectors_per_block;


void simple_memcpy(uint8_t* dest, uint8_t* src, uint32_t count)
{
    for (uint32_t i = 0; i < count; i++) {
        dest[i] = src[i];
    }
}

// offset : offset in the block (0 = start of block)
// returns the number of bytes read
uint32_t read_block(uint32_t block, uint32_t offset, uint32_t count, uint8_t* buf)
{
    // special case : sparse block 
    if (block == 0) {
        int i = 0;
        while (i < count && offset + i < block_size) {
            buf[i] = 0;
            i++;
        }
        return i;
    }

    uint32_t buf_offs = 0;
    uint8_t* tmp_buf = (uint8_t*)TMP_LOAD_ADDR_1;

    for (uint32_t i = 0; i < sectors_per_block; i++) {
        // start and end (in bytes) of the sector
        // relative to the start of the block
        uint32_t start = i * SECTOR_SIZE;
        uint32_t end = start + SECTOR_SIZE;

        if (start < offset + count && offset < end) {
            // bytes to skip (beginning) and trim (end) in the sector we will read
            uint32_t skip = 0;
            if (start < offset) {
                skip = offset - start;
            }
            uint32_t trim = 0;
            if (offset + count < end) {
                trim = end - (offset + count);
            }

            ata_read_sector(block * sectors_per_block + i, tmp_buf);
            simple_memcpy(buf + buf_offs, tmp_buf + skip, SECTOR_SIZE - skip - trim);
            buf_offs += SECTOR_SIZE - skip - trim;
        }
    }
    return buf_offs;
}

uint32_t max(uint32_t a, uint32_t b)
{
    return a < b ? b : a;
}

// offset into the file
// returns 0 if all went fine
int read_file(void* inode, uint32_t offset, uint32_t count, uint8_t* buf)
{
    uint32_t buf_offs = 0;
    // block of the first byte to read
    uint32_t first = offset / block_size;
    // block of the last byte to read
    uint32_t last = (offset + count - 1) / block_size;

    // direct blocks 
    for (uint32_t i = first; i < EXT2_DIRECT_BLOCKS && i <= last; i++) {
        uint32_t block = *((uint32_t*)(inode + 40 + 4*i));
        // start of the range of bytes the block represents in the file
        uint32_t start = i * block_size;
        uint32_t ofs = (start < offset) ? (offset - start) : 0;
        uint32_t cnt = (start < offset) ? count : ((offset + count) - start);
        buf_offs += read_block(block, ofs, cnt, buf + buf_offs);
    }

    // indirect blocks
    uint32_t* indir_blocks = (uint32_t*)TMP_LOAD_ADDR_2;
    read_block(*((uint32_t*)(inode+88)), 0, 0xFFFFFFFF, (uint8_t*)indir_blocks);
    for (uint32_t i = max(first, EXT2_DIRECT_BLOCKS); 
        i - EXT2_DIRECT_BLOCKS < block_size / sizeof(uint32_t) && i <= last; 
        i++) 
    {
        uint32_t block = indir_blocks[i - EXT2_DIRECT_BLOCKS];
        // start of the range of bytes the block represents in the file
        uint32_t start = i * block_size;
        uint32_t ofs = (start < offset) ? (offset - start) : 0;
        uint32_t cnt = (start < offset) ? count : (offset + count) - start;
        buf_offs += read_block(block, ofs, cnt, buf + buf_offs);
    }

    if (buf_offs < count) {
        return -1;
    }
    return 0;
}

bool valid_header(Elf32_Ehdr* e_hdr)
{
    if (e_hdr->e_ident[EI_MAG0] == 0x7f &&
        e_hdr->e_ident[EI_MAG1] == 'E' &&
        e_hdr->e_ident[EI_MAG2] == 'L' &&
        e_hdr->e_ident[EI_MAG3] == 'F')
    {
        return true;
    }
    return false;
}

void load_segments(void* kernel_inode, Elf32_Ehdr* e_hdr)
{
    // load segments
    for (uint32_t i = 0; i < e_hdr->e_phnum; i++) {
        Elf32_Phdr header; // on the stack
        read_file(
            kernel_inode, 
            e_hdr->e_phoff + i * e_hdr->e_phentsize, 
            sizeof(Elf32_Phdr), 
            &header
        );
        if (header.p_type == PT_LOAD) {
            // copy the segment
            // it is the kernel's responsibility to not 
            // load itself on top of the bootloader
            // i.e. to load itself after 1MB 
            read_file(
                kernel_inode, 
                header.p_offset, 
                header.p_filesz, 
                (uint8_t*)header.p_paddr
            );
            // rest of the segment is filled with zero
            uint8_t* ptr = header.p_paddr;
            for (int i = header.p_filesz; i < header.p_memsz; i++) {
                *(ptr + i) = 0;
            }
        }
    }
}

// returns 0 if all went fine
int load_kernel(void* kernel_inode, void* boot_info)
{
    Elf32_Ehdr e_hdr; // on the stack
    read_file(kernel_inode, 0, sizeof(Elf32_Ehdr), &e_hdr);

    if (!valid_header(&e_hdr)) {
        return -1;
    }
    // we can only load an executable file
    if (e_hdr.e_type != ET_EXEC) {
        return false;
    }

    load_segments(kernel_inode, &e_hdr);

    // by convention, for the kernel executable, the e_entry field 
    // is the PHYSICAL address of the entry point 
    extern void jump_to_kernel(uint32_t entry_point, void* boot_info);
    jump_to_kernel(e_hdr.e_entry, boot_info);

    return -1; // should not happen
}