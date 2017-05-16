#ifndef _SYS_H
#define _SYS_H

#include<linux/sched.h>

extern int sys_fork();
extern int sys_pause();
extern int stub_syscall();
extern int serial_debugstr(char *str);
extern int sys_kill(int pid, int sig);

// 目前除了少数syscall之外其余的syscall均为stub状态
fn_ptr sys_call_table[] = {
    stub_syscall, // 0
    stub_syscall,
    sys_fork,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall, // 10
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall, // 20
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    sys_pause,
    stub_syscall, // 30
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    sys_kill,
    stub_syscall,
    stub_syscall,
    stub_syscall, // 40
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall, // 50
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall, // 60
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    serial_debugstr,
};

#endif
