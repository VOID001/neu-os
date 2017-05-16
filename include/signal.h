#ifndef _SIGNAL_H
#define _SIGNAL_H

#include <sys/types.h>

typedef int sig_atomic_t;
typedef unsigned int sigset_t;

#define _NSIG 32		// 32种信号，所以下面的信号集也是32bit啦
#define NSIG _NSIG

// 定义信号宏

#define SIGHUP 		1
#define SIGINT 		2
#define SIGQUIT 	3
#define SIGILL 		4
#define SIGTRAP 	5
#define SIGABRT 	6
#define SIGIOT 		6
#define SIGUNUSED 	7
#define SIGFPE 		8
#define SIGKILL 	9
#define SIGUSR1 	10
#define SIGSEGV		11
#define SIGUSR2		12
#define SIGPIPE		13
#define SIGALRM		14
#define SIGSTKFLT	16
#define SIGCHLD		17
#define SIGCONT		18
#define SIGSTOP		19
#define SIGTSTP		20
#define SIGTTIN		21
#define SIGTTOU		22

// 下面定义 sigaction 需要的结构
#define SA_NOCLDSTOP 1
#define SA_NOMASK 0x40000000		// 表示不阻止在某个信号的信号处理程序中再次收到该信号
#define SA_ONESHOT 0x80000000		// 只调用一次信号处理句柄

#define SIG_BLOCK 0
#define SIG_UNBLOCK 1
#define SIG_SETMASK 2

#define SIG_DFL ((void(*) (int))0)		// 默认信号处理句柄
#define SIG_IGN ((void(*) (int))1)		// 信号处理忽略句柄

void (*signal(int _sig, void(*func)(int)))(int);
int raise(int sig);						// 向自身发信号
int kill(pid_t pid, int sig);			// 向某个进程发信号

typedef unsigned int sigset_t;  // 32bit 信号集，一位表示一个信号，对于linux0.11已经够用了

struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
};

// 对阻塞信号集的操作
// Not implemented in sigaction
int sigaddset(sigset_t *mask, int signo);
int sigdelset(sigset_t *mask, int signo);
int sigemptyset(sigset_t *mask);
int sigfillset(sigset_t *mask);
int sigismember(sigset_t *mask, int signo);

int sigpending(sigset_t *set);
int sigprocmask(int how, sigset_t *set, sigset_t *oldset);
// 改变对于某个信号sig的处理过程
int sigaction(int sig, struct sigaction *act, struct sigaction *oldact);

#endif
