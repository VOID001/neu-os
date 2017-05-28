/*
 * Rewrite printk
 * now printk call for sprintf
 * then call tty_write
 * (user mode will call write(x, x, x)
 */
#include <stdarg.h>
#include <stddef.h>
#include <linux/tty.h>
#include <serial_debug.h>

void vsprintf(char *dest, char *fmt, ...);

void printk(char *fmt, ...) {
    int i = 0;
    char buf[TTY_BUF_SIZE] = "";
    char *ptr = buf;
    va_list ap;
    for(i = 0; i < TTY_BUF_SIZE; i++) {
        buf[i] = 0;
    }
    s_printk("[DEBUG] printk called, fmt = %s", fmt);
    va_start(ap, fmt); 
    vsprintf(buf, fmt, ap);
    va_end(ap);
    while(*ptr) {
        tty_push_q(&tty_table[0].write_q, *ptr);
        tty_queue_stat(&tty_table[0].write_q);
        ptr++;
    }
    tty_write(&tty_table[0]);
}


