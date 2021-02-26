#include <stdio.h>
#ifdef __is_libk
#include "vga_driver.h"
#endif 


int puts(const char* str)
{
#ifdef __is_libk
    vga_print(str);
    vga_print("\n");
    return 1;
#endif
    return EOF;
}