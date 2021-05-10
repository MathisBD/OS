#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <stdbool.h>

#ifdef __is_libk
#include "threads/file_descr.h"
#define OPEN    kopen
#define WRITE   kwrite
#define CLOSE   kclose
#define FD      file_descr_t*
#endif 
#ifdef __is_libc
#include <user_file.h>
#define OPEN    open
#define WRITE   write
#define CLOSE   close
#define FD      
#endif


#define MAX_BASE 16
static char digits[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
};

// returns the number of digits written into buf
// the maximum number of digits written should be 64 (in base 2)
static int sprint_unsigned(uint64_t i, int base, char* buf);

static int sprint_signed(int64_t i, int base, char* buf)
{
    if (base < 2 || base > MAX_BASE) {
        return 0;
    }
    if (i < 0) {
        buf[0] = '-';
        return 1 + sprint_unsigned((uint64_t)(-i), base, buf + 1);
    }
    else {
        return sprint_unsigned((uint64_t)i, base, buf);
    }
}

static int sprint_unsigned(uint64_t i, int base, char* buf)
{
    if (base < 2 || base > MAX_BASE) {
        return 0;
    }
    if (i < base) {
        buf[0] = digits[i];
        return 1;
    }

    int c = sprint_unsigned(i / base, base, buf);
    buf[c] = digits[i % base];
    return c + 1;
}

// 10**9-1 <= max_uint32 = 2**32-1
#define PARSE_UINT32_MAX_DIGITS 9
// parses a uint32 in decimal
// if no uint32 is read, doesn't change num
// returns the number of characters read
static int parse_uint32(const char* buf, uint32_t* num)
{
    uint32_t res = 0;
    
    int i = 0;
    while (i < PARSE_UINT32_MAX_DIGITS &&'0' <= buf[i] && buf[i] <= '9') {
        res = 10 * res + (buf[i] - '0');
        i++;
    }

    if (i > 0) {
        *num = res;
    }
    return i;
}

static void print_char(char c, FD file)
{
    WRITE(file, &c, 1);
}


int printf(const char* __restrict format, ...)
{
    FD file = OPEN("/dev/vga", FD_PERM_WRITE);

    va_list params;
	va_start(params, format);
    
    size_t i = 0;
    size_t written = 0;
    char num_buf[64]; // used to write numbers into
    while (format[i]) {
        // double percent
        if (!memcmp(format + i, "%%", 2)) {
            print_char('%', file);
            written++;
            i += 2;
        }
        // variables
        else if (!memcmp(format + i, "%", 1)) {
            i += 1;

            // parse a width specifier : e.g. %4x pads with leading spaces
            bool pad_zeros = false; // if true, pad with zeros instead of spaces
            if (format[i] == '0') {
                pad_zeros = true;
                i += 1;
            }
            uint32_t pad_width = 0;
            i += parse_uint32(format + i, &pad_width);

            // contents to write
            char* contents = num_buf; // reset this every iteration
            size_t contents_size = 0;

            if (!memcmp(format + i, "c", 1)) {
                // we can't use a type smaller than int
                // for va_arg (i.e. char won't do)
                int c = va_arg(params, int);
                contents[0] = (char)c;
                contents_size = 1;
                i += 1;
            }
            else if (!memcmp(format + i, "s", 1)) {
                const char * str = va_arg(params, const char *);
                contents = str;
                contents_size = strlen(str);
                i += 1;
            }
            // %d
            else if (!memcmp(format + i, "d", 1)) {
                int n = va_arg(params, int);
                contents_size = sprint_signed(n, 10, contents);
                i += 1;
            }
            else if (!memcmp(format + i, "ld", 2)) {
                long int n = va_arg(params, long int);
                contents_size = sprint_signed(n, 10, contents);
                i += 2;
            }
            else if (!memcmp(format + i, "lld", 3)) {
                long long int n = va_arg(params, long long int);
                contents_size = sprint_signed(n, 10, contents);
                i += 3;
            }
            // %u
            else if (!memcmp(format + i, "u", 1)) {
                unsigned int n = va_arg(params, unsigned int);
                contents_size = sprint_unsigned(n, 10, contents);
                i += 1;
            }
            else if (!memcmp(format + i, "lu", 2)) {
                unsigned long int n = va_arg(params, unsigned long int);
                contents_size = sprint_unsigned(n, 10, contents);
                i += 2;
            }
            else if (!memcmp(format + i, "llu", 3)) {
                unsigned long long int n = va_arg(params, unsigned long long int);
                contents_size = sprint_unsigned(n, 10, contents);
                i += 3;
            }
            // %x
            else if (!memcmp(format + i, "x", 1)) {
                unsigned int n = va_arg(params, unsigned int);
                contents_size = sprint_unsigned(n, 16, contents);
                i += 1;
            }
            else if (!memcmp(format + i, "lx", 2)) {
                unsigned long int n = va_arg(params, unsigned long int);
                contents_size = sprint_unsigned(n, 16, contents);
                i += 2;
            }
            else if (!memcmp(format + i, "llx", 3)) {
                unsigned long long int n = va_arg(params, unsigned long long int);
                contents_size = sprint_unsigned(n, 16, contents);
                i += 3;
            }
            else {
                // bad variable formatter
                CLOSE(file);
                return EOF; 
            }

            // actually print
            if (contents_size < pad_width) {
                for (int i = 0; i < pad_width - contents_size; i++) {
                    if (pad_zeros) {
                        print_char('0', file);
                    }
                    else {
                        print_char(' ', file);
                    }
                }
            }
            WRITE(file, contents, contents_size);
        }
        // normal characters
        else {
            print_char(format[i], file);
            written++;
            i++;
        }
    }
    
    va_end(params);
    //CLOSE(file);
    return written;
}