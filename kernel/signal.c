#include <linux/kernel.h>
#include <signal.h>
#include <linux/types.h>
#include <serial_debug.h>
#include <asm/segment.h>
#include <linux/sched.h>

#define DEBUG

// do_signal 函数调用之前内核栈的参数分布，见文档

void dump_sigaction(struct sigaction *action) {
    s_printk("[DEBUG] Sigaction dump\n");
    s_printk("[DEBUG] addr = 0x%x, sa_mask = 0x%x, sa_handler = 0x%x, sa_restorer = 0x%x\n", 
            action, action->sa_mask, action->sa_handler, action->sa_restorer);
}

void do_signal(long signr, long eax, long ebx, long ecx,
        long edx, long fs, long es, long ds,
        unsigned long eip, long cs, long eflags, unsigned long *esp, long ss) {
#ifdef DEBUG
    s_printk("[DEBUG] Signal = %d\n", signr);
    s_printk("[DEBUG] Context signr = %d, eax = 0x%x, ebx = 0x%x, ecx = 0x%x\n \
            edx = 0x%x, fs = 0x%x, es = 0x%x, ds = 0x%x\n \
            eip = 0x%x, cs = 0x%x, eflags = 0x%x, esp = 0x%x, ss= 0x%x\n",
            signr, eax, ebx, ecx, edx, fs, es, ds, eip, cs, eflags ,esp, ss);
    s_printk("[DEBUG] current pid = %d\n", current->pid);
    dump_sigaction(&current->sigaction[signr - 1]);
#endif
    unsigned long sa_handler;
    unsigned long old_eip = eip;
    struct sigaction *sa = current->sigaction + signr - 1;
    unsigned int longs;
    unsigned long *tmp_esp;

    sa_handler = (unsigned long)sa->sa_handler;
    // IGNORE HANDLER
    if (sa_handler == 1) {
        return ;
    }
    // DEFAULT HANDLER
    if (!sa_handler) {
        if(signr == SIGCHLD)
            return;
        else
            panic("Default signal handler");
    }
    // User registered signal handler, then process
    if ((sa->sa_flags | SA_ONESHOT)) {
        sa->sa_handler = NULL;
    }
    // Make EIP in the stack pointed to handler function
    *(&eip) = sa_handler;
    // Check wether we need to push sigmask
    longs = (sa->sa_flags & SA_NOMASK)?7:8;
    // Grow the stack
    *(&esp) -= longs;

    verify_area((void *)esp, longs * 4);
    tmp_esp = (unsigned long *)esp;
    put_fs_long((unsigned long)sa->sa_restorer, tmp_esp++);
    put_fs_long((unsigned long)signr, tmp_esp++);
    if (!(sa->sa_flags & SA_NOMASK))
        put_fs_long((unsigned long)current->blocked, tmp_esp++);
    put_fs_long((unsigned long)eax, tmp_esp++);
    put_fs_long((unsigned long)ecx, tmp_esp++);
    put_fs_long((unsigned long)edx, tmp_esp++);
    put_fs_long((unsigned long)eflags, tmp_esp++);
    put_fs_long((unsigned long)old_eip, tmp_esp++);
    current->blocked |= sa->sa_mask;
}

int sys_sgetmask() {
    return (int)current->blocked;
}

int sys_ssetmask(int newmask) {
    int old;
    old = (int)current->blocked;
    // We cannot mask SIGKILL
    current->blocked = (unsigned long)(newmask & ~(1<<(SIGKILL-1)));
    return old;
}

// Save old first verify the area has enough space
// and correct permission
static inline void save_old(char *from, char *to) {
    unsigned int i;
    verify_area((void *)to, sizeof(struct sigaction));
    for (i = 0; i < sizeof(struct sigaction); i++) {
        put_fs_byte(*from, to);
        from++;
        to++;
    }
}

static inline void get_new(char *from, char *to) {
    unsigned int i;
    for (i = 0; i < sizeof(struct sigaction); i++) {
        *to = get_fs_byte(from);
        from++;
        to++;
    }
}

// set signal handler in old styled POSIX way
int sys_signal(int signum, long handler, long restorer) {
    struct sigaction tmp;
    // Validate the signal
    if (signum < 1 || signum > 32 || signum == SIGKILL) {
        return -1;
    }
    tmp.sa_handler = (void (*)(int))handler;
    tmp.sa_mask = 0;
    tmp.sa_flags = (int)(SA_ONESHOT | SA_NOMASK);
    tmp.sa_restorer = (void (*)(void))restorer;
    handler = (long) current->sigaction[signum-1].sa_handler;
    current->sigaction[signum - 1] = tmp;
    return handler;
}

int sys_sigaction(int signum, const struct sigaction* action, 
        struct sigaction *old_action) {
    struct sigaction tmp;
#ifdef DEBUG
    s_printk("[DEBUG] sys_sigaction entered, signum = %d\n", signum);
#endif
    if (signum < 1 || signum > 32 || signum == SIGKILL)
        return -1;
    tmp = current->sigaction[signum - 1];
#ifdef DEBUG
    s_printk("[DEBUG] sigaction MARK %d\n", __LINE__);
#endif
    get_new((char *)action, (char *)&(current->sigaction[signum - 1]));
    if (old_action)
        save_old((char *) &tmp, (char *)old_action);
    if (current->sigaction[signum - 1].sa_flags & SA_NOMASK)
        current->sigaction[signum - 1].sa_mask = 0;
    else
        current->sigaction[signum - 1].sa_mask |= (sigset_t)(1<<(signum - 1));
#ifdef DEBUG
    s_printk("[DEBUG] current pid = %d\n", current->pid);
    dump_sigaction(&current->sigaction[signum-1]);
#endif
    return 0;
}


#undef DEBUG
