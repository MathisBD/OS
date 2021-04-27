#include <stdio.h>
#include <stdint.h>


#ifdef __is_libk
int print_mem(const void* _buf, size_t count)
{
    uint8_t* buf = _buf;
    for (size_t i = 0; i < count; i++) {
        printf("%02x", buf[i]);

        if (i % 16 == 15) {
            printf("\n");
        }
        else if (i % 8 == 7) {
            printf("   ");
        }
        else {
            printf(" ");
        }
    }
    return 0;
}
#endif
