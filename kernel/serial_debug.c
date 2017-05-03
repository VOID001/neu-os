#include <asm/io.h>
#include <stdarg.h>
#define PORT 0x3f8

int is_transmit_empty() {
    return inb(PORT + 5) & 0x20;
}

void s_putchar(char a) {
    while (is_transmit_empty() == 0);

    outb(PORT,a);
}

void s_puts(char *a) {
    while(*a) {
        s_putchar(*a++);
    }
}

int serial_debugstr(char *str) {
    s_puts("Called!");
    s_puts(str);
    return 0;
}

void s_printnum(int num, int base, int sign) {
    char digits[] = "0123456789ABCDEF";
    char buf[50] = "";
    int cnt = 0;
    int i;

    if(sign && num < 0) {       // Check for sign or unsign
        s_putchar('-');
        num = -num;
    }

    if(num == 0) {
        s_putchar('0');
        return ;
    }
    while(num) {
        buf[cnt++] = digits[num % base];
        num = num / base;
    }

    for(i = cnt - 1; i >=0; i--) {
        s_putchar(buf[i]);
    }
    return ;
}



void s_printk(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);

    char c, *s;

    while(*fmt) {
        c = *fmt++;
        if(c != '%') {
            s_putchar(c);
            continue;
        }
        c = *fmt++;
        if(c == '\0')
            break;
        switch(c) {
            case 'd':
                s_printnum(va_arg(ap, int), 10, 1);
                break;
            case 'u':
                s_printnum(va_arg(ap, int), 10, 0);
                break;
            case 'x':
                s_printnum(va_arg(ap, int), 16, 0);
                break;
            case 's':
                s = va_arg(ap, char*);
                while(*s)
                    s_putchar(*s++);
                break;
            case '%':
                s_putchar('%');
        }
    }
    return;
}

void s_printldt(const char* ldt) {

}
