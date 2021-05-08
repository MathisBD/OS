#include "threads/file_descr.h"



file_descr_t* kopen(char* name);
void kclose(file_descr_t*);
int kwrite(file_descr_t*, void* buf, uint32_t count);
int kread(file_descr_t*, void* buf, uint32_t count);
void kpipe(file_descr_t* from, file_descr_t* to);
void kdup(file_descr_t* from, file_descr_t* to);
