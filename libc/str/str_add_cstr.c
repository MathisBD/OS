#include <str.h>
#include <string.h>


void str_add_cstr(str_t* str, const char* cstr)
{
    uint32_t n = strlen(cstr);
    for (int i = 0; i < n; i++) {
        str_add_char(str, cstr[i]);
    }
}