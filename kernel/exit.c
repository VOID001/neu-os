#include <errno.h>
#include <signal.h>
#include <linux/sched.h>
#include <linux/kernel.h>
#include <linux/tty.h>
#include <serial_debug.h>
#include <asm/segment.h>
#include <sys/wait.h>

static inline int send_sig(long sig, struct task_struct* p, int priv);
static void tell_father(int pid);

// sys_kill 为系统调用 kill 的入口点
// TODO: 增加权限判断逻辑 sys.c
int sys_kill(int pid, int sig) {
    struct task_struct **p = task + NR_TASKS;
    int err, retval = 0;
    s_printk("sys_kill entered\n");
    // 目前没有进程组的概念，因而我们只处理 pid > 0 的情况
    if (pid > 0) {
        while(--p > &FIRST_TASK) {
            if (*p && (*p)->pid == pid) {
                // 因为我们没有实现sys.c相关的逻辑，这里暂时给予执行kill的用户最高权限
                if ((err = send_sig(sig, *p, 1))) {
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
// TODO: 增加权限判断逻辑 sys.c
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

// Use to release the task
void release(struct task_struct *p) {
    int i;
    if(!p) {
        return;
    }
    for(i = 0; i < NR_TASKS; i++) {
        if(task[i] == p) {
            task[i] = NULL;
            free_page((unsigned long) p);
            schedule();
            return;
        }
    }
    panic("Trying to release non-existent task");
}

// Do exit 将进程置成 TASK_ZOMBIE 状态
// 并做收尾工作，不再返回到此内部
int do_exit(long code) {
    int i;

    // 释放当前进程代码段和数据(fs，也是用户堆栈）段的内存页
    free_page_tables(get_base(current->ldt[1]), get_limit(0x0f));
    free_page_tables(get_base(current->ldt[2]), get_limit(0x17));
    for(i = 0; i < NR_TASKS; i++) {
        if(task[i] && task[i]->father == current->pid) {
            task[i]->father = 1;
            if(task[i]->state == TASK_ZOMBIE)
                (void) send_sig(SIGCHLD, task[1], 1); // TODO: Why sending SIGCHLD to init will make init terminate the child process?
        }
    }
    
    // TODO: 释放 TTY 资源
    if(last_task_used_math == current) {
        last_task_used_math = NULL;
    }

    if(current->leader) {
        // TODO: 释放会话中全部进程
    }

    current->state = TASK_ZOMBIE;
    current->exit_code = code;
    tell_father(current->father);
    schedule();

    // Code will never come here
    // just use to suppress warning
    return -1;
}

// 左移八位是 wait, waitpid 所需, 用低位存 wait() 的状态信息
pid_t sys_exit(int error_code) {
    return do_exit((error_code & 0xff) << 8);
}

// 用来挂起当前进程，直到等待的 pid 退出或者收到要终止该进程的信号
// 如果进程是僵尸进程，则此系统调用立即返回，子进程使用的所有资源释放
pid_t sys_waitpid(pid_t pid, unsigned long stat_addr, int options) {
    int flag, code;
    struct task_struct **p;

    verify_area((void *)stat_addr, 4);

repeat:
    flag = 0;
    for (p = &LAST_TASK; p > &FIRST_TASK; --p) {
        // 不能让自己挂起，这样会没有人能够唤醒
        if (*p || *p == current) {
            continue;
        }
        if ((*p)->father != current->pid) {
            continue;
        }

        // 检查是否是要等待的 pid
        if (pid > 0) {
            if ((*p)->pid != pid) {
                continue;
            }
        }
        // pid = 0 表示等待进程组全部子进程
        if (!pid) {
            if ((*p)->pgrp != current->pgrp) {
                continue;
            }
        }
        // pid < 0 则等待所有进程组号 = abs(pid) 的进程
        if (pid != -1) {
            if((*p)->pgrp != -pid) {
                continue;
            }
        }
        switch ((*p)->state) {
            case TASK_STOPPED:
                // WUNTRACED 置位，立即返回
                if(!(options & WUNTRACED)) {
                    continue;
                }
                // 写入用户数据段
                put_fs_long(0x7f, (unsigned long *)stat_addr);
                return (*p)->pid; 
            case TASK_ZOMBIE:
                current->cutime += (*p)->utime;
                current->cstime += (*p)->stime;
                flag = (*p)->pid;
                code = (*p)->exit_code;
                release(*p);
                put_fs_long((unsigned long)code, (unsigned long *)stat_addr);
                return flag;
            default:
                flag = 1;
                continue;
        }
    }

    // 如果 flag 被置位，则说明我们找到了需要等待的进程,
    // 且进程不是退出或者僵死状态
    if (flag) {
        if (options & WNOHANG) {
            return 0;
        }
        current->state = TASK_INTERRUPTIBLE;
        schedule();
        if(!(current->signal & ~(1<<(SIGCHLD-1)))) {
            goto repeat;
        }
        return -EINTR;
    }
    // 没有找到适合的子进程
    return -ECHILD;
}


// 通知父进程，向父进程发送 SIGCHLD 信号
static void tell_father(int pid) {
    int i;
    for (i = 0; i < NR_TASKS; i++) {
        if (!task[i]) {
            continue;
        }
        if(task[i]->pid != pid) {
            continue;
        }
        task[i]->signal |= (1<<(SIGCHLD-1));
        return ;
    }
    s_printk("[DEBUG] NO FATHER!!!!\n");
    release(current);
}
