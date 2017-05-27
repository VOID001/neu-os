#include <linux/head.h>
#include <linux/sched.h>
#include <linux/tty.h>
#include <serial_debug.h>


// 定义对 tty_queue 的操作
// tty_queue 是一个循环队列，操作要注意这一点

int tty_isempty_q(const struct tty_queue q) {
    if (q.head == q.tail)
        return 1;
    return 0;
}

int tty_isfull_q(const struct tty_queue q) {
    if ((q.tail + 1) % TTY_BUF_SIZE == q.head)
        return 1;
    return 0;
}

char tty_pop_q(struct tty_queue *q) {
    char ch;
    ch = q->buf[q->head];
    q->head = (q->head + 1) % TTY_BUF_SIZE;
    return ch;
}

// 队列满状态下返回 -1
int tty_push_q(struct tty_queue *q, char ch) {
    if (tty_isfull_q(*q))
        return -1;
    q->buf[q->tail] = ch;
    q->tail = (q->tail + 1) % TTY_BUF_SIZE;
    return 0;
}

void tty_queue_stat(const struct tty_queue *q) {
    unsigned int i;
    s_printk("[DEBUG] Queue head = %d, tail = %d\n", q->head, q->tail);
    s_printk("[DEBUG] Queue: [ ");
    for (i = q->head; i != q->tail; i = (i + 1) % TTY_BUF_SIZE) {
        s_printk("%c<-", q->buf[i]);
    }
    s_printk(" ]\n");
}
