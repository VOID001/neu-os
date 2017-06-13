#include <unistd.h>
#include <linux/head.h>
#include <linux/sched.h>
#include <serial_debug.h>

// #define DEBUG

extern int sys_alarm(long seconds);

int stub_syscall(void) {
    return 0;
}

// Here we implement syscall for sleep
int sys_sleep(long seconds) {
#ifdef DEBUG
    s_printk("[DEBUG] syscall entered, sleeping\n");
#endif
    sys_alarm(seconds);
    current->state = TASK_INTERRUPTIBLE;
    schedule();
    return 0;
}
