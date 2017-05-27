#ifndef _TTY_H
#define _TTY_H

#include <linux/sched.h>
#define TTY_BUF_SIZE 1024

#define TTY_ECHO 0x0001     // 回显标志

void tty_init();

struct tty_queue {
    char buf[TTY_BUF_SIZE];
    struct task_struct *wait_proc;      // 等待该缓冲区的进程队列
    unsigned long head;
    unsigned long tail;
};

struct tty_struct {
    void (*write)(struct tty_struct *tty);
    int pgrp;
    unsigned int flags;
    struct tty_queue read_q;
    struct tty_queue write_q;
};

extern struct tty_struct tty_table[];
int tty_isempty_q(const struct tty_queue q);
int tty_isfull_q(const struct tty_queue q);
char tty_pop_q(struct tty_queue *q);
int tty_push_q(struct tty_queue *q, char ch);
void tty_queue_stat(const struct tty_queue *q);

#endif
