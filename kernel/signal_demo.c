#define __LIBRARY__
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <linux/kernel.h>

// First we define kill system call

_syscall2(int, kill, int, pid, int, sig)
static inline _syscall1(int, sys_debug, char *, str)
static inline _syscall3(int, sigaction, int, signum, struct sigaction *, action,
        struct sigaction *, old_action)

extern void __sig_restore();
extern void __masksig_restore();

// A function use to demo signal usage
//

static void demo_handle(int sig) {
    sys_debug("Demo handler activate!\n");
    sig++;
}


void signal_demo_main(void) {
    int ret = 0;
    struct sigaction sa_action;
    sa_action.sa_handler = demo_handle;
    // sa_action.sa_flags |= SA_NOMASK; // TODO: Add mask
    sa_action.sa_mask = 0;
    sa_action.sa_restorer = __sig_restore;
    ret = sigaction(SIGUSR1, &sa_action, NULL);
    if (ret) {
        panic("sigaction set failed");
    }
    kill(1, SIGUSR1);
    return;
}

