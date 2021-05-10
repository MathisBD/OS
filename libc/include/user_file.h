#pragma once
#include <stdint.h>


#define FD_PERM_READ    0x1
#define FD_PERM_WRITE   0x2

#define FD_STDIN        0
#define FD_STDOUT       1

// file descriptor id
typedef uint32_t fid_t;

fid_t open(char* name, uint8_t perms);
void close(fid_t file);
int write(fid_t file, void* buf, uint32_t count);
int read(fid_t file, void* buf, uint32_t count);
void pipe(fid_t* from, fid_t* to);
void dup(fid_t from, fid_t to);
