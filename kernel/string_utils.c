#include "string_utils.h"

// reverses in place
void reverse_str(char * str) {
    int size = 0;
    while (str[size] != 0) {
        size++;
    }

    for (int i = 0; i < size - i - 1; i++) {
        char tmp = str[i];
        str[i] = str[size - i - 1];
        str[size - i - 1] = tmp; 
    }
}

bool int_to_string(uint64_t num, char * str, int str_size)
{
    if (str_size <= 1) {
        return false;
    }

    if (num == 0) {
        str[0] = '0';
        str[1] = 0;
        return true;
    }

    int i = 0;
    while (num > 0) {
        str[i++] = '0' + (num % 10);
        num = num / 10;

        if (i+1 >= str_size) {
            return false;
        }
    }
    str[i] = 0;

    reverse_str(str);
    return true;
}