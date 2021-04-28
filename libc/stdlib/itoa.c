#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>

static char digits[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'A', 'B', 'C', 'D', 'E', 'F'
};

void itoa_unsigned(uint32_t value, char* str, int base)
{
    uint32_t ofs = 0;
    while (value > 0) {
        str[ofs] = digits[value % base];
        value /= base;
        ofs++;
    }

    // reverse the string
    for (uint32_t i = 0; 2*i < ofs; i++) {
        char tmp = str[i];
        str[i] = str[ofs-i-1];
        str[ofs-i-1] = tmp;
    }
    str[ofs] = 0;
}

char* itoa(int value, char* str, int base)
{
    if (base == 10 && value < 0) {
        str[0] = '-';
        itoa_unsigned(-value, str+1, base);
    }
    else {
        itoa_unsigned(value, str, base);
    }
    return str;
}