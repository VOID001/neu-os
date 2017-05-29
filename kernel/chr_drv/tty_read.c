#define __LIBRARY__
#include <unistd.h>

// Define user_tty_read syscall for test
_syscall3(int ,user_tty_read, int, channel, char *, buf, int, nr)
