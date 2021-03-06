#pragma once
#include <stdint.h>


#define FD_PERM_READ    0x1
#define FD_PERM_WRITE   0x2

#define FD_STDIN        0
#define FD_STDOUT       1


#define FILE_TYPE_STREAM_DEV    1
#define FILE_TYPE_FILE          2
// 3 would be pipes, but they have no names
#define FILE_TYPE_DIR           4
#define FILE_TYPE_ERR           5 // e.g. the file doesn't exist

// file descriptor id
typedef uint32_t fid_t;

fid_t open(char* name, uint8_t perms);
void close(fid_t file);
int write(fid_t file, void* buf, uint32_t count);
int read(fid_t file, void* buf, uint32_t count);
void seek(fid_t file, uint32_t ofs, uint8_t flags);

void pipe(fid_t* from, fid_t* to);
fid_t dup(fid_t original);
// makes copy be a copy of original. 
// closes copy first if necessary.
void dup2(fid_t original, fid_t copy);

fid_t create(char* path, uint8_t file_type);
void remove(char* path, uint8_t file_type);
uint32_t file_type(char* path);

uint32_t get_size(fid_t file);
// returns the previous size
uint32_t resize(fid_t file, uint32_t size);

// lists the names of the files in the directory,
// and puts them into buf, separated with null terminators.
// returns 0 if we could fit the whole directory in the buffer,
// otherwise returns the number of characters written to the buffer
// (and a negative value means error).
int list_dir(fid_t dir, void* buf, uint32_t size);

void chdir(const char* path);
// returns 0 if buf was big enough and > 0 if there wasn't enough space.
int getcwd(void* buf, uint32_t size);
