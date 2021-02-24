#ifdef __is_libk
#include "vga_driver.h"
#endif 

void printf(char* str)
{
#ifdef __is_libk
    vga_print(str);
#endif 
}