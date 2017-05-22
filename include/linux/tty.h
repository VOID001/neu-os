#ifndef _TTY_H
#define _TTY_H

#include <termios.h>

#define TTY_BUF_SIZE 1024

struct tty_queue {
    unsigned long data;
    unsigned long head;
    unsigned long tail;
    struct task_struct *proc_list;
    char buf[TTY_BUF_SIZE];
};

// 这里的顺序不能直接更改，会导致 rs_io.s kb.S 中的 offset 出问题
// 他们需要一起来改
struct tty_struct {
    struct termios termios;     
    int pgrp;
    int stopped;
    void (*write)(struct tty_struct *tty);
    struct tty_queue read_q;
    struct tty_queue write_q;
    struct tty_queue secondary;
};

extern struct tty_struct tty_table[];

// 定义 tty_queue 队列操作宏
// 移动队列指针，如果超过队列长度就循环
#define INC(a) ((a) = ((a) + 1) & (TTY_BUF_SIZE - 1))
#define DEC(a) ((a) = ((a) - 1) & (TTY_BUF_SIZE - 1))
#define EMPTY(queue) ((queue).head = (queue).tail)
#define LEFT(queue) (((queue).tail - (queue).head - 1) & (TTY_BUF_SIZE - 1))
#define LAST(queue) ((queue).buf[(TTY_BUF_SIZE-1)&((queue).head - 1)])
#define FULL(queue) (!LEFT(queue))
#define GETCH(queue, c) \
(void) ({c = (queue).buf[(queue).tail]; INC((queue).tail);})
#define PUTCH(c, queue) \
(void) ({(queue).buf[(queue).head]=(c); INC((queue).head);})
#define CHARS(queue) (((queue).head - (queue).tail) & (TTY_BUF_SIZE - 1))

#define INTR_CHAR(tty) ((tty)->termios.c_cc[VINTR])
#define QUIT_CHAR(tty) ((tty)->termios.c_cc[VQUIT])
#define ERASE_CHAR(tty) ((tty)->termios.c_cc[VERASE])
#define KILL_CHAR(tty) ((tty)->termios.c_cc[VKILL])
#define EOF_CHAR(tty) ((tty)->termios.c_cc[VEOF])
#define START_CHAR(tty) ((tty)->termios.c_cc[VSTART])
#define STOP_CHAR(tty) ((tty)->termios.c_cc[VSTOP])
#define SUSPEND_CHAR(tty) ((tty)->termios.c_cc[VSUSP])

#define INIT_C_CC "\003\034\177\025\004\0\1\0\021\023\032\0\022\017\027\026\0"

void rs_init();
void tty_init();
void con_init();

int tty_read(unsigned c, char *buf, int n);
int tty_write(unsigned c, char *buf, int n);

void rs_write(struct tty_struct *tty);
void con_write(struct tty_struct *tty);
void copy_to_cooked(struct tty_struct *tty);

#endif
