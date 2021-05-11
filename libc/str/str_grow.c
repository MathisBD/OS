#include <str.h>
#include <string.h>


void str_grow(str_t* str, uint32_t cap)
{
    if (cap <= str->capacity) {
        return;
    }

    while (str->capacity < cap) {
        str->capacity *= 2;
    }
    void* new_buf = MALLOC(str->capacity);
    memset(new_buf, 0, str->capacity);
    memcpy(new_buf, str->buf, str->size);
    FREE(str->buf);
    str->buf = new_buf;
}