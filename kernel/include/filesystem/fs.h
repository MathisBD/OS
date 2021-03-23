#pragma once 
#include <stdint.h>

// This is the layer between the ext2 filesystem and the kernel/user
// Kind of like a VFS except it can only support EXT2
// In particular code in this file should not know any details
// about ext2 implementation, only what is in ext2.h

// the given inode (dir or file) (specified by a number 
// or by a path) does not exist
#define FS_ERR_INODE_EXIST     (-1)
#define FS_ERR_READ            (-2)
#define FS_ERR_WRITE           (-3)
#define FS_ERR_NO_SPACE        (-4)
// the reasons for this failure are not meant 
// to be known by the user (e.g. they depend on 
// ext2 low level details). Ideally this error
// should never happen
#define FS_ERR_INTERN_FAIL     (-5)
#define FS_ERR_INODE_TYPE      (-6)
// the path given by the user has an incorrect format
#define FS_ERR_PATH_FORMAT     (-7)
// the given directory is non empty
#define FS_ERR_DIR_NONEMPTY    (-8)
// read after the end of the file
#define FS_ERR_FILE_BOUNDS     (-9)

// the maximum length the name of file/dir can be
#define FS_MAX_NAME_LEN   255 // because it must fit on 8 bits for ext2

// these numbers are arbitrary
#define FS_INODE_TYPE_DIR       1
#define FS_INODE_TYPE_FILE      2

typedef struct dir_entry_ {
    char* name;
    uint32_t inode;
    struct dir_entry_* next;
} dir_entry_t;

// path format : '/' separated names, with a leading '/'
// example : '/dir1/dir2/file.txt' is valid
// the root path : '/' is not valid (the functions declared here
// have no reason to operate on the root)

int init_fs();

// assumes the parent directory exists
int make_dir(const char* path);
// assumes the directory exists and is empty
int rem_dir(const char* path);

// assumes the parent directory exists
int make_file(const char* path);
// assumes the file exists
int rem_file(const char* path);

int find_inode(const char* path, uint32_t* inode);
int inode_type(uint32_t inode, uint32_t* type);
int file_size(uint32_t file, uint32_t* size);

// can't read past the file end
int read_file(uint32_t file, uint32_t offset, uint32_t count, void* buf);
// automatically grows the file if writting past the bounds
int write_file(uint32_t file, uint32_t offset, uint32_t count, void* buf);
int resize_file(uint32_t file, uint32_t size);

int list_dir(uint32_t dir, dir_entry_t** entries);