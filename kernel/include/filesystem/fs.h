#pragma once 
#include <stdint.h>
#include "filesystem/ext2/ext2.h"

// This is the layer between the ext2 filesystem and the kernel/user
// Kind of like a VFS except it can only support EXT2
// In particular code in this file should not know any details
// about ext2 implementation, only what is in ext2.h



void make_dir(const char* path);
void rem_dir(const char* path);

void list_dir(const char* path);

// assumes the parent directory exists
void make_file(const char* path);
// assumes the file exists
void remove_file(const char* path);

void read_file(uint32_t file, uint32_t offset, uint32_t count, void* buf);
// automatically grows the file if writting past the bounds
void write_file(uint32_t file, uint32_t offset, uint32_t count, void* buf);
void resize_file(uint32_t file, uint32_t size);