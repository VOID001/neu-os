#ifndef _UNISTD_H
#define _UNISTD_H

#ifdef __LIBRARY__

#define __NR_fork 2
#define __NR_pause 29
#define __NR_sys_debug 72

#define _syscall0(type, name)

#define _syscall1(type, name, atype, a)

#define _syscall2(type, name, atype, a, btype, b)

#define _syscall3(type, name, atype, a, btype, b, ctype, c)

#endif

static int pause(void);

#endif
