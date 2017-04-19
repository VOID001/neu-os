#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/system.h>
#include <asm/io.h>

union task_union {
    struct task_struct task;
    char stack[PAGE_SIZE];
};
static union task_union init_task = {INIT_TASK,};

long user_stack[PAGE_SIZE >> 2];
long startup_time;
struct task_struct *current = &(init_task.task);

struct {
    long *a;
    short b;
} stack_start = {&user_stack[PAGE_SIZE >> 2], 0x10};

void sleep_on(struct task_struct **p) {

}

void schedule(void) {

}

void wake_up(struct task_struct **p) {

}

void interruptible_sleep_on(struct task_struct **p) {

}

int sys_pause(void) {

}

// 计时器函数 do_timer, 记录程序运行时间，如果处于 CPL = 2 的进程执行时间超过时间片
// 则进行调度，否则继续。

// 下面的这段代码是一个demo，用来验证时钟中断
// 已经设置好并且可用
int counter = 0;
long volatile jiffies = 0;
void do_timer(long cpl) {
    //jiffies++;
    counter++;
    if(counter == 10){
        printk("CPL = %d Jiffies = %d\n", cpl, jiffies);
        counter = 0;
    }
}

void demo_timer_interrupt(void);
void timer_interrupt(void);

// 这是一个临时函数，用于初始化8253计时器
// 并开启时钟中断
void timer_init() {
    int divisor = 1193180/HZ;
    outb_p(0x43, 0x36);
    outb_p(0x40, divisor & 0xFF);
    outb_p(0x40, divisor >> 8);

    // timer interrupt gate setup: INT 0x20
    set_intr_gate(0x20, &timer_interrupt);
    // Make 8259 accept timer interrupt
    outb(0x21, inb_p(0x21) & ~0x01);
}
