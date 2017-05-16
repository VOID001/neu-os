#define __LIBRARY__
#include <signal.h>
#include <unistd.h>
#include <errno.h>

// First we define kill system call

_syscall2(int, kill, int, pid, int, sig)
static inline _syscall1(int, sys_debug, char *, str)

// A function use to demo signal usage
//

void signal_demo_main(void) {
    kill(2, SIGSEGV);
    return;
}
