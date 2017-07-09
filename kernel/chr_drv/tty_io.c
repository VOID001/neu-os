#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>
#include <asm/system.h>
#include <asm/segment.h>
#include <asm/io.h>
#include <serial_debug.h>
#include <linux/tty.h>

#define DEBUG

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
        // TODO Judge if it's a normal character
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
            case -1:
            // We do not process \n here, process it in tty_read
            case '\n':
                s_printk("Enter wake the tty read queue up!\n");
                tty_push_q(buffer, ch);
                wake_up(&tty_table[0].buffer.wait_proc);
                break;
                // EOF
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

// 队列为空时，进程进入睡眠状态, 可被中断唤醒
// TODO: write sleep_if_empty
void sleep_if_empty(struct tty_queue *q) {
    cli();
    while(!current->signal && tty_isempty_q(q))
        interruptible_sleep_on(&q->wait_proc); 
    sti();
}

static void sleep_if_full(struct tty_queue *q) {
    cli();
    while(!current->signal && tty_isfull_q(q))
        interruptible_sleep_on(&q->wait_proc); 
    sti();
}

// tty_read 函数从 tty->buffer 中读取内容
// 当没有读到足够的字符且 buffer 为空的时候
// sleep
int tty_read(int channel, char *buf, int nr) {
    int len = 0;
    char ch;
    char *p = buf;
    char tmpbuf[TTY_BUF_SIZE];
    int tmpbuf_len = 0;
#ifdef DEBUG
    s_printk("[TTY] buf addr = 0x%x\n", buf);
#endif

    // TODO make channel adjustable
    channel = 0;
    if (channel > 2 || channel < 0 || nr < 0)
        return -1;
    struct tty_struct *tty = tty_table + channel;
    // Sleep until the queue is not empty
    // Only we get an \n, we start to process
    // interruptible_sleep_on(&tty->buffer.wait_proc);
    //
    // outer loop for reading lines
    while (1) {
        // If the queue is empty and we haven't finish reading
        // Then we sleep, only enter can wake up the sleep
        // sleep_if_empty(&tty->buffer);

        // If enter triggered, we start to read then back
        while(1) {
            ch = tty_pop_q(&tty->buffer);
#ifdef DEBUG
            if(tty_isempty_q(&tty->buffer)) {
                s_printk("Empty!");
            }
            tty_queue_stat(&tty->buffer);
#endif
            if (nr <= 0) {
                // get the char then put it back
                tmpbuf[tmpbuf_len++] = ch;
                if (ch == '\n' || ch == -1) {
                    for (int i = tmpbuf_len - 1; i >= 0; i--) {
                        tty_push_q_front(&tty->buffer, tmpbuf[i]);
                    }
                    break;
                    // revert the queue and stop the cycle
                }
                // put the char in tmp queue and continue
                continue;
            }
            // TODO: Change -1 to EOF
            // We also keep the \n
            if (ch == '\n' || ch == -1) {
                // TODO: Figure out why put_fs_byte don't work
                // put_fs_byte(ch, p++);
                *p++ = ch;
                len++;
                nr--;
                break;
            }
            // put_fs_byte(ch, p++);
            *p++ = ch;
            len++;
            nr--;
        }
#ifdef DEBUG
        s_printk("Buf = %s\n", buf);
#endif
        if (nr <= 0) {
            break;
        }
    }

    return len;
}

int _user_tty_write(int channel, char *buf, int nr) {
    int i = 0;
    // TODO make channel adjustable
    channel = 0;
    struct tty_struct *tty = tty_table + channel;
    for (i = 0; i < nr; i++) {
        tty_push_q(&tty->write_q, buf[i]);
    }
    tty_write(tty);
    // TODO return len
    return nr;
}
