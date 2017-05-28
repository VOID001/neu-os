#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>
#include <asm/system.h>
#include <asm/io.h>
#include <serial_debug.h>
#include <linux/tty.h>

void con_init();
extern void con_write(struct tty_struct *tty);

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
        },
        .buffer = {
            .head = 0,
            .tail = 0
        }
    },
};

// struct tty_queue read_q;
void tty_init(void) {
    tty_table[0].write = con_write;
    con_init();
}

// copy_to_buffer handle the keyboard input
// to tty echo char
void copy_to_buffer(struct tty_struct *tty) {
    char ch;
    struct tty_queue *read_q= &tty->read_q;
    struct tty_queue *buffer= &tty->buffer;
    while(!tty_isempty_q(&tty->read_q)) {
        ch = tty_pop_q(read_q); 
        // Judge if it's a normal character
        // if (ch < 32) {
        //     // This is control character
        //     continue;
        // }
        switch(ch) {
            case '\b':
                // This is backspace char
                if (!tty_isempty_q(buffer)) {
                    if(tty_queue_tail(buffer) == '\n')  // \n 不能被清除掉
                       continue ;
                    buffer->tail = (buffer->tail - 1) % TTY_BUF_SIZE;
                } else {
                    continue;
                }
                break;
            case '\n':
            default:
                if (!tty_isfull_q(buffer)) {
                    tty_push_q(buffer, ch);
                } else {
                    // here we need to sleep until the queue
                    // is not full
                }
                break;
        }

        if (tty->flags | TTY_ECHO) {
            tty_push_q(&tty->write_q, ch);
            tty->write(tty);
        }
    }
    return ;
}

void tty_write(struct tty_struct* tty) {
    tty->write(tty);
    return ;
}


