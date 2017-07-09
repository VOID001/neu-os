#ifndef _SYS_H
#define _SYS_H

#include<linux/sched.h>

extern int sys_fork();
extern int sys_pause();
extern int stub_syscall();
extern int serial_debugstr(char *str);
extern int sys_kill(int pid, int sig);
extern int sys_sigaction(int signum, struct sigaction *action, struct sigaction *old_action);
extern int sys_sgetmask(void);
extern int sys_ssetmask(int newmask);
// Just for debug use
extern int tty_read(int channel, char *buf, int nr);
extern int _user_tty_write(int channel, char *buf, int nr);
extern int sys_alarm(long seconds);
extern int sys_sleep(long seconds);

// 目前除了少数syscall之外其余的syscall均为stub状态
fn_ptr sys_call_table[] = {
    tty_read, // 0
    stub_syscall,
    sys_fork,
    _user_tty_write, // 3
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    stub_syscall,
    sys_sleep, // 10
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
    sys_sigaction,
    sys_sgetmask,
    sys_ssetmask,
    stub_syscall,
    stub_syscall,
    serial_debugstr,
};

#endif
