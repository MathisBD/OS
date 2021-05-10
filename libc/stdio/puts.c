#include <stdio.h>

#ifdef __is_libk
#include "drivers/vga_driver.h"
#include "threads/file_descr.h"
#include "init/init.h"
#include <string.h>
int puts(const char* str)
{
    char nl = '\n';
    file_descr_t* fd = kopen("/dev/vga", FD_PERM_WRITE);
    kwrite(fd, str, strlen(str));
    kwrite(fd, &nl, 1);
    //kclose(fd);
    return 1;
}
#endif

#ifdef __is_libc
#include <string.h>
#include <user_file.h>
int puts(const char* str)
{
    char nl = '\n';
    file_descr_t* fd = open("/dev/vga");
    write(fd, str, strlen(str));
    write(fd, &nl, 1);
    //close(fd);
    return 1;
}
#endif