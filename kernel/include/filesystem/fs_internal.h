#include "filesystem/fs.h"
#include <stdbool.h>

// that's how it is on ext2 filesystems
#define ROOT_INODE  2

// assume the child name length is valid (i.e. not zero and doesn't exceed
// the maximum size allowed)
int dir_add_child(uint32_t dir, const char* child_name, uint32_t child);
int dir_rem_child(uint32_t dir, const char* child_name);
int dir_find_child(uint32_t dir, const char* child_name, uint32_t* child);