#include <stdio.h>

#ifdef __is_libk
#include "drivers/vga_driver.h"
#include "threads/file_descr.h"
int putchar(int ic)
{
    file_descr_t* fd = kopen("/dev/vga", FD_PERM_WRITE);
    kwrite(fd, &ic, 1);
    //kclose(fd);
    return ic;
}
#endif

#ifdef __is_libc
#include <user_file.h>
int putchar(int ic)
{
    file_descr_t* fd = open("/dev/vga");
    write(fd, &ic, 1);
    //close(fd);
    return ic;
}
#endif