/*
 * User mode printf function
 */
#define __LIBRARY__
#include <stdarg.h>
#include <linux/tty.h>
#include <stddef.h>
#include <unistd.h>
#include <serial_debug.h>
 
extern int user_tty_write(int channel, char *buf, int nr);
void vsprintf(char *dest, char *fmt, va_list ap);

static inline int sys_debug(char *str);

static inline _syscall1(int, sys_debug, char *, str)

int printf(char *fmt, ...) {
    int i = 0;
    char buf[TTY_BUF_SIZE] = "";
    char *ptr = buf;
    va_list ap;
    for(i = 0; i < TTY_BUF_SIZE; i++) {
        buf[i] = 0;
    }
    va_start(ap, fmt); 
    vsprintf(buf, fmt, ap);
    va_end(ap);
    i = 0;
    while(*ptr) {
        i++;
        ptr++;
    }
    return user_tty_write(0, buf, i);
}
