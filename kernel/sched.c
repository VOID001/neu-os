#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/mm.h>
#include <asm/system.h>

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
