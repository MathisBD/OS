#pragma once


#if defined(__is_kernel) || defined(__is_libk)
void panic(const char* msg);
#endif 
