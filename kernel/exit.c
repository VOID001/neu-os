#include <errno.h>
#include <signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/tty.h>

static inline int send_sig(long sig, struct task_struct* p, int priv);

// sys_kill 为系统调用 kill 的入口点
int sys_kill(int pid, int sig) {
    struct task_struct **p = task + NR_TASKS;
    int err, retval = 0;
    s_printk("sys_kill entered\n");
    // 目前没有进程组的概念，因而我们只处理 pid > 0 的情况
    if (pid > 0) {
        while(--p > &FIRST_TASK) {
            if (*p && (*p)->pid == pid) {
                // 因为我们没有实现sys.c相关的逻辑，这里暂时给予执行kill的用户最高权限
                if (err = send_sig(sig, *p, 1)) {
                    s_printk("send_sig error err = %d\n", err);
                    retval = err;
                }
            }
        }
    }
    s_printk("sys_kill leaved, retval = %d\n", retval);
    return retval;
}

// send_sig 用来向进程发送信号
static inline int send_sig(long sig, struct task_struct* p, int priv) {
    // s_printk("send_sig entered\n");
    // First check params
    if (!p || sig < 1 || sig > 32)
        return -EINVAL;
    if(priv || current->euid == p->euid /*|| suser() */) // 目前我们没有对用户的权限检查
        p->signal |= (1<<(sig - 1));
    else
        return -EPERM;
    return 0;
}
