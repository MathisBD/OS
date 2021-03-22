#include "filesystem/fs.h"



void parse_path(const char* path, char** names);
void dir_add_child(uint32_t dir, const char* child_name, uint32_t child);
uint32_t dir_rem_child(uint32_t dir, uint32_t )
uint32_t dir_find_child(uint32_t dir, const char* child_name);