#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>
#include <asm/system.h>
#include <asm/io.h>
#include <serial_debug.h>
#include <linux/tty.h>

extern void keyboard_interrupt(char scancode);

// 这里我们用 C99 的 dot initializer 初始化
struct tty_struct tty_table[] = {
    {
        .pgrp = 0,
        .write = NULL,
        .flags = 0 | TTY_ECHO,
        .read_q = {
            .head = 0,
            .tail = 0,
        },
        .write_q = {
            .head = 0,
            .tail = 0,
        }
    },
};

// struct tty_queue read_q;

void con_init(void) {
    register unsigned char a;
    set_trap_gate(0x21, &keyboard_interrupt);
    outb_p(0x21, inb_p(0x21)&0xfd);
    a = inb_p(0x61);
    outb_p(0x61, a | 0x80);
    outb(0x61, a);
}

void tty_init(void) {
    // read_q.head = 0;
    // read_q.tail = 0;
    // s_printk("{%d %d %d}", &read_q, read_q.head, read_q.tail);
    // tty_queue_stat(&read_q);
    con_init();
}
