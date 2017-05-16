#include <linux/kernel.h>
#include <signal.h>
#include <linux/types.h>
#include <serial_debug.h>

// do_signal 函数调用之前内核栈的参数分布，见文档
int do_signal(long signr, long eax, long ebx, long ecx,
        long edx, long fs, long es, long ds,
        long eip, long cs, long eflags, long esp, long ss) {
    s_printk("Stubbed signal hanlder\n");
    s_printk("Signal = %d\n", signr);
    return 0;
}
