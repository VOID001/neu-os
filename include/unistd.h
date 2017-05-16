#ifndef _UNISTD_H
#define _UNISTD_H

#ifdef __LIBRARY__

#define __NR_fork 2
#define __NR_pause 29
#define __NR_kill 37
#define __NR_sys_debug 72

#define _syscall0(type, name)  \
type name(void) \
{ \
    long __res; \
    __asm__ volatile("int $0x80\n\t" \
            : "=a" (__res)          \
            : "0" (__NR_##name)); \
    if (__res >= 0) \
        return (type) __res; \
    /*errno = -__res;*/ \
    return  -1;\
}

#define _syscall1(type, name, atype, a) \
type name(atype a) \
{ \
    long __res; \
    __asm__ volatile("int $0x80\n\t" \
            : "=a" (__res) \
            : "0" (__NR_##name), "b" ((long) a)); \
    if (__res >= 0) \
        return (type) __res; \
    /*errno = -__res;*/ \
    return -1; \
}

#define _syscall2(type, name, atype, a, btype, b) \
type name(atype a, btype b) \
{ \
    long __res; \
    __asm__ volatile("int $0x80\n\t" \
            : "=a" (__res) \
            : "0" (__NR_##name), "b" ((long) a), "c" ((long) b)); \
    if (__res >= 0) \
        return (type) __res; \
    /*errno = -__res;*/ \
    return -1; \
}

#define _syscall3(type, name, atype, a, btype, b, ctype, c) \
type name(atype a, btype b, ctype c) \
{ \
    long __res; \
    __asm__ volatile("int $0x80\n\t" \
            : "=a" (res) \
            : "0" (__NR_##name), "b" ((long) a), "c" ((long) c), "d" ((long) c)); \
    if (__res >= 0) \
        return (type) __res; \
    /* errno = -__res;*/ \
    return -1; \
}

#endif

static int pause(void);

#endif
