#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>


#define MAX_BASE 16
static char digits[] = {
    '0', '1', '2', '3', '4', '5', '6', '7', '8', '9',
    'a', 'b', 'c', 'd', 'e', 'f'
};

int print_unsigned(uint64_t i, int base);

int print_signed(int64_t i, int base)
{
    if (base < 2 || base > MAX_BASE) {
        return 0;
    }
    if (i < 0) {
        putchar('-');
        return 1 + print_unsigned((uint64_t)(-i), base);
    }
    else {
        return print_unsigned((uint64_t)i, base);
    }
}

int print_unsigned(uint64_t i, int base)
{
    if (base < 2 || base > MAX_BASE) {
        return 0;
    }
    if (i < base) {
        return putchar(digits[i]);
    }

    int c = print_unsigned(i / base, base);
    c += putchar(digits[i % base]);
    return c;
}

int printf(const char* __restrict format, ...)
{
    va_list params;
	va_start(params, format);
    
    size_t i = 0;
    size_t written = 0;
    while (format[i]) {
        // double percent
        if (!memcmp(format + i, "%%", 2)) {
            written += putchar('%');
            i += 2;
        }
        // variables
        else if (!memcmp(format + i, "%", 1)) {
            i += 1;
            // room for parsing format specifiers here
            // e.g. : %-8d
            if (!memcmp(format + i, "c", 1)) {
                // we can't use a type smaller than int
                // for va_arg (i.e. char won't do)
                int c = va_arg(params, int);
                written += putchar(c);
                i += 1;
            }
            else if (!memcmp(format + i, "s", 1)) {
                const char * str = va_arg(params, const char *);
                size_t len = strlen(str);
                for (int k = 0; k < len; k++) {
                    written += putchar(str[k]);
                }
                i += 1;
            }
            // %d
            else if (!memcmp(format + i, "d", 1)) {
                int n = va_arg(params, int);
                written += print_signed(n, 10);
                i += 1;
            }
            else if (!memcmp(format + i, "ld", 2)) {
                long int n = va_arg(params, long int);
                written += print_signed(n, 10);
                i += 2;
            }
            else if (!memcmp(format + i, "lld", 3)) {
                long long int n = va_arg(params, long long int);
                written += print_signed(n, 10);
                i += 3;
            }
            // %u
            else if (!memcmp(format + i, "u", 1)) {
                unsigned int n = va_arg(params, unsigned int);
                written += print_unsigned(n, 10);
                i += 1;
            }
            else if (!memcmp(format + i, "lu", 2)) {
                unsigned long int n = va_arg(params, unsigned long int);
                written += print_unsigned(n, 10);
                i += 2;
            }
            else if (!memcmp(format + i, "llu", 3)) {
                unsigned long long int n = va_arg(params, unsigned long long int);
                written += print_unsigned(n, 10);
                i += 3;
            }
            // %x
            else if (!memcmp(format + i, "x", 1)) {
                unsigned int n = va_arg(params, unsigned int);
                written += print_unsigned(n, 16);
                i += 1;
            }
            else if (!memcmp(format + i, "lx", 2)) {
                unsigned long int n = va_arg(params, unsigned long int);
                written += print_unsigned(n, 16);
                i += 2;
            }
            else if (!memcmp(format + i, "llx", 3)) {
                unsigned long long int n = va_arg(params, unsigned long long int);
                written += print_unsigned(n, 16);
                i += 3;
            }
            else {
                // bad variable formatter
                return EOF; 
            }
        }
        // normal characters
        else {
            written += putchar(format[i]);
            i++;
        }
    }
    
    va_end(params);
    return written;
}