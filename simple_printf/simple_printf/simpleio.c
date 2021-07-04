#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "simpleio.h"

static void print_int_recursive(unsigned int val)
{
    if (val == 0) {
        return;
    }

    print_int_recursive(val / 10);
    putchar('0' + val % 10);
}

void printf_simple(const char* format, ...)
{
    va_list ap;

    const char* pFormat = format;

    va_start(ap, format);
    {
        while (*pFormat != '\0') {
            unsigned val;
            const char* str;

            switch (*pFormat) {
                case 's':
                    str = va_arg(ap, const char*);
                    while (*str != '\0') {
                        putchar(*str++);
                    }
                    break;
                case 'c':
                    val = va_arg(ap, unsigned int);
                    putchar(val);
                    break;
                case 'd':
                {
                    int value = va_arg(ap, int);
                    if (value < 0) {
                        putchar('-');
                        print_int_recursive(abs(value));
                    }
                    else if (value == 0)
                    {
                        putchar('0');
                    }
                    else
                    {
                        print_int_recursive(value);
                    }
                    break;
                }
                default:
                    putchar(*pFormat);
                    break;
            }

            ++pFormat;
        }
    }
    va_end(ap);

    va_start(ap, format);
    {
        while (*format != '\0') {
            unsigned val;
            const char* str;

            switch (*format) {
            case 's':
                str = va_arg(ap, const char*);
                while (*str != '\0') {
                    putchar(*str++);
                }
                break;
            case 'c':
                val = va_arg(ap, unsigned int);
                putchar(val);
                break;
            case 'd':
            {
                int value = va_arg(ap, int);
                if (value < 0) {
                    putchar('-');
                    print_int_recursive(abs(value));
                }
                else if (value == 0)
                {
                    putchar('0');
                }
                else
                {
                    print_int_recursive(value);
                }
                break;
            }
            default:
                putchar(*format);
                break;
            }

            ++format;
        }
    }
    va_end(ap);
}
