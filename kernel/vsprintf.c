#include <stdarg.h>
#include <stddef.h>
#include <serial_debug.h>

// Implement sprintf for print formatted string
// Convert it to a string

int _sprintnum(char *dest, int num, int base, int sign);
void vsprintf(char *dest, char *fmt, va_list ap) {
    // va_start(ap, fmt);   va_start 只能使用一次，获得 va_list 之后就不要 start 啦, 不然就会出现参数输出全都不对的错误

    char c, *s;
    char *dest_ptr = dest;

    while(*fmt) {
        c = *fmt++;
        if(c != '%') {
            *(dest_ptr++) = c;
            continue;
        }
        c = *fmt++;
        if(c == '\0')
            break;
        switch(c) {
            case 'd':
                dest_ptr = dest_ptr + _sprintnum(dest_ptr, va_arg(ap, int), 10, 1);
                break;
            case 'u':
                dest_ptr = dest_ptr + _sprintnum(dest_ptr, va_arg(ap, int), 10, 0);
                break;
            case 'x':
                dest_ptr = dest_ptr + _sprintnum(dest_ptr, va_arg(ap, int), 16, 0);
                break;
            case 's':
                s = va_arg(ap, char*);
                while(*s)
                    *dest_ptr++ = (*s++);
                break;
            case 'c':
                *dest_ptr++ = (va_arg(ap, char));
                break;
            case '%':
                *dest_ptr++ = ('%');
        }
    }
    return;
}

int _sprintnum(char *dest, int num, int base, int sign) {
    char digits[] = "0123456789ABCDEF";
    char buf[50] = "";
    int cnt = 0;
    int len = 0;
    int i;

    if(sign && num < 0) {       // Check for sign or unsign
        *dest++ = '-';
        num = -num;
        len++;
    }

    if(num == 0) {
        *dest++= '0';
        len++;
        return len;
    }
    while(num) {
        buf[cnt++] = digits[num % base];
        num = num / base;
    }

    for(i = cnt - 1; i >=0; i--) {
        *dest++ = (buf[i]);
        len++;
    }
    return len;
}
