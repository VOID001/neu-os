#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <linux/sys.h>
#include <asm/system.h>
#include <asm/io.h>
#include <serial_debug.h>

extern int timer_interrupt(void);
extern int system_call(void);

union task_union {
    struct task_struct task;
    char stack[PAGE_SIZE];
};

static union task_union init_task = {INIT_TASK,};

long user_stack[PAGE_SIZE >> 2];
long startup_time;
struct task_struct *current = &(init_task.task);
struct task_struct *last_task_used_math = NULL;
struct task_struct *task[NR_TASKS] = {&(init_task.task), };

struct {
    long *a;
    short b;
} stack_start = {&user_stack[PAGE_SIZE >> 2], 0x10};

void sleep_on(struct task_struct **p) {
    struct task_struct *tmp;

    if(!p)
        return;
    if(current == &(init_task.task))
        panic("task[0] trying to sleep");
    tmp = *p;
    *p = current;
    current->state = TASK_UNINTERRUPTIBLE;
    schedule();
    *p = tmp;
    if (tmp)
        tmp->state = TASK_RUNNING;
}

void schedule(void) {
    // 我们先不考虑信号处理
    int i, next, c;
    struct task_struct **p;

    while(1) {
        // 初始化，i, p指向任务链表的末尾
        c = -1;
        next = 0;
        i = NR_TASKS;
        p = &task[NR_TASKS];
        while(--i) {
            // 跳过无效任务（空）
            if(!*(--p))
                continue;
            if((*p)->state == TASK_RUNNING && (*p)->counter > c) {
                c = (*p)->counter;
                next = i;
            }
        }
        // 如果循环之后，系统中的任务只有counter > 0的或者没有任何可以
        // 运行的任务，那么就退出循环并进行任务切换
        if(c) break;
        for( p = &LAST_TASK; p > &FIRST_TASK; p--) {
            if(!*p) {
                (*p)->counter = ((*p)->counter >> 1) + (*p)->priority;
            }
        }
    }
    // switch_to 接收参数为一个 Task 号
    // show_task_info(task[next]);
    // s_printk("Scheduler select task %d\n", next);
    switch_to(next)
}

void show_task_info(struct task_struct *task) {
    s_printk("Current task Info\n================\n");
    s_printk("pid = %d\n", task->state);
    s_printk("counter = %d\n", task->counter);
    s_printk("start_code = %x\n", task->start_code);
    s_printk("end_code = %x\n", task->end_code);
    s_printk("brk = %x\n", current->ldt[0]);
    s_printk("gid = 0x%x\n", current->gid);
    s_printk("tss.ldt = 0x%x\n", current->tss.ldt);
    // s_printk("tss.eip = 0x%x\n", current->eip);
}

void wake_up(struct task_struct **p) {
    if(p && *p) {
        (*p)->state = TASK_RUNNING;
        *p = NULL;
    }
}

// 假设我们执行任务A的时候调用了这个函数
void interruptible_sleep_on(struct task_struct **p) {
    struct task_struct *tmp;
    
    if(!p)
        return;
    if(current == &(init_task.task)) // 我们不能让 init sleep
        panic("task[0] trying to sleep");
    tmp = *p;
    *p = current;
rep_label: current->state = TASK_INTERRUPTIBLE;
    // 这里会转到其他任务去执行
    schedule(); 
    // 回来的时候说明，调度程序调度到了这里，current = B
    // 我们要检查队列，如果 *p （这里应该是A) 和 当前运行任务不等
    // 如果不是的话
    if(*p && *p != current) {
        (*p)->state = TASK_RUNNING;
        goto rep_label;
    }
    *p = tmp;
    if(tmp)
        tmp->state = TASK_RUNNING;
}

int sys_pause(void) {
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    return 0;
}

// 计时器函数 do_timer, 记录程序运行时间，如果处于 CPL = 2 的进程执行时间超过时间片
// 则进行调度，否则继续。

// 下面的这段代码是一个demo，用来验证时钟中断
// 已经设置好并且可用, 目前不支持其他定时器
// floppy操作依旧不支持
int counter = 0;
long volatile jiffies = 0;
void do_timer(long cpl) {
    if (!cpl)
        current->stime++;
    else
        current->utime++;
    if((--current->counter) > 0) return ;
    current->counter = 0;
    if(!cpl) return;
    schedule();
}

// 这是一个临时函数，用于初始化8253计时器
// 并开启时钟中断
void sched_init() {
    int divisor = 1193180/HZ;

    int i;
    struct desc_struct *p;

    // 初始化任务0的TSS, LDT
    set_tss_desc(gdt + FIRST_TSS_ENTRY, &(init_task.task.tss));
    set_ldt_desc(gdt + FIRST_LDT_ENTRY, &(init_task.task.ldt));

    // 初始化其余各项,因为TSS LDT各占一个，所以
    // +2 指向任务1的TSS, LDT(初始任务为0)
    // 把其余各项设置为0
    p = gdt + 2 + FIRST_TSS_ENTRY;
    for(i = 1; i < NR_TASKS; i++) {
        task[i] = NULL;
        p->a = p->b = 0;
        p++;
        p->a = p->b = 0;
        p++;
    }

    // Clear Nested Task(NT) Flag
    __asm__("pushfl; andl $0xffffbfff, (%esp); popfl");
    ltr(0);
    lldt(0);
    
    outb_p(0x43, 0x36);
    outb_p(0x40, divisor & 0xFF);
    outb_p(0x40, divisor >> 8);

    // timer interrupt gate setup: INT 0x20
    set_intr_gate(0x20, &timer_interrupt);
    // Make 8259 accept timer interrupt
    outb(0x21, inb_p(0x21) & ~0x01);

    // 初始化 system_call
    set_system_gate(0x80, &system_call);
}
