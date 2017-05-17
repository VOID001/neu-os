static inline char get_fs_byte(const char *addr) {
    register char _v;
    __asm__ volatile("movb %%fs:%1, %0":"=r" (_v):"m"(*addr));
    return _v;
}

static inline unsigned short get_fs_word(const char *addr) {
    unsigned short _v;
    __asm__ volatile("movw %%fs:%1, %0":"=r" (_v):"m"(*addr));
    return _v;
}

static inline unsigned long get_fs_long(const char *addr) {
    unsigned long _v;
    __asm__ volatile("movl %%fs:%1, %0":"=r" (_v):"m"(*addr));
    return _v;
}

static inline void put_fs_byte(char val, char *addr) {
    __asm__ volatile("movb %0, %%fs:%1":: "r" (val), "m" (*addr));
}

static inline void put_fs_word(short val, short *addr) {
    __asm__ volatile("movw %0, %%fs:%1":: "r" (val), "m" (*addr));
}

static inline void put_fs_long(unsigned long val, unsigned long *addr) {
    __asm__ volatile("movl %0, %%fs:%1"::  "r" (val), "m" (*addr));
}
