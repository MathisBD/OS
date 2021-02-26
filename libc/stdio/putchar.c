#include <stdio.h>
#ifdef __is_libk
#include "vga_driver.h"
#endif 

int putchar(int ic)
{
#ifdef __is_libk 
    vga_putchar((char)ic);
#endif 
    return ic;
}