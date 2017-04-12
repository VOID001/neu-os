#ifndef _SIGNAL_H
#define _SIGNAL_H

typedef unsigned int sigset_t;  // 32bit 信号集，一位表示一个信号，对于linux0.11已经够用了

struct sigaction {
    void (*sa_handler)(int);
    sigset_t sa_mask;
    int sa_flags;
    void (*sa_restorer)(void);
}

#endif
