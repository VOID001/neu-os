//#include <errno.h> 当前所有错误号为stub状态
#include <linux/sched.h>
#include <asm/system.h>
#include <linux/mm.h>
//#include <linux/sys.h>
#include <linux/kernel.h>

extern void write_verify(unsigned long address);

long last_pid = 0;

// 对齐内存区域，并且对区域进行验证，区域以4096Byte
// 为单位, 进行write_verify(因为当时的CPU还不支持WP位)
void verify_area(void *addr, unsigned int size) {
    unsigned long start;

    start = (unsigned long)addr;
    size += start & 0xfff;      // size align
    start &= 0xfffff000;        // start align at 4096Byte
    start += get_base(current->ldt[2]);
    while(size) {
        size -= 4096;
        write_verify(start);
        start += 4096;
    }
}

int copy_mem(int nr, struct task_struct *p) {
    unsigned long old_data_base, new_data_base, data_limit;
    unsigned long old_code_base, new_code_base, code_limit;

    code_limit = get_limit(0x0f);
    data_limit = get_limit(0x17);
    old_code_base = get_base(current->ldt[1]);
    old_data_base = get_base(current->ldt[2]);
    if(old_code_base != old_data_base) // This should never happen
        panic("Codeseg not fully overlapped with dataseg");
    if(data_limit < code_limit)
        panic("bad data limit");

    new_data_base = new_code_base = (unsigned int)nr * 0x4000000; // 64MB per nr
    p->start_code = new_code_base;
    set_base(p->ldt[1], new_code_base);
    set_base(p->ldt[2], new_data_base);
    if(copy_page_tables(old_data_base, new_data_base, data_limit)) {
        printk("free_page_tables: from copy_mem\n");
        free_page_tables(new_data_base, data_limit);
        return -1;
    }
    return 0;
}

// 这里拷贝PCB，以及代码段数据段, 首先我们需要知道，C 语言中参数的获取顺序
// 在函数参数列表中的参数，顺序是这样的，参数列表最后面的参数，对应
// 在汇编中最先压栈的参数（就是离栈顶最远的参数)
// 然后这里的参数涉及到很多，解释一下为什么会有这些参数
// 首先 CPU 进入中断调用，压栈SS, ESP, 标志寄存器 EFLAGS 和返回地址CS:EIP
// 然后是system_call里压栈的寄存器 ds, es, fs, edx, ecx, ebx
// 然后是调用sys_fork函数时,压入的函数返回地址
// 以及sys_fork里压栈的那些参数 gs, esi, edi, ebp, eax(nr)
int copy_process(int nr, long ebp, long edi, long esi,
        long gs, long none, long ebx, long ecx, long edx, long fs,
        long es, long ds, long eip, long cs, long eflags, long esp, long ss) {
    struct task_struct *p;
    int i;

    /* Only for emit the warning! */
    i = none;
    none = i;
    i = eflags;
    eflags = i;
    /* End */
    
    p = (struct task_struct *) get_free_page();
    if(!p)
        return -1;
    task[nr] = p;
    *p = *current; // 这里仅仅复制PCB(task_struct)

    // 初始化 PCB, TSS
    p->state = TASK_UNINTERRUPTIBLE;
    p->pid = last_pid;
    p->father = current->pid;
    p->counter = p->priority;
    p->signal = 0;
    p->alarm = 0;
    p->leader = 0;
    p->utime = p->stime = 0;
    p->cutime = p->cstime = 0;
    p->start_time = jiffies;
    p->tss.back_link = 0;
    p->tss.esp0 = PAGE_SIZE + (long)p;
    p->tss.ss0 = 0x10;
    p->tss.eip = eip;
    p->tss.eax = 0; // 这是当fork返回的时候，子进程返回0的原因
    p->tss.ecx = ecx;
    p->tss.ebx = ebx;
    p->tss.edx = edx;
    p->tss.esp = esp;
    p->tss.ebp = ebp;
    p->tss.esi = esi;
    p->tss.edi = edi;
    p->tss.es = es & 0xffff;
    p->tss.cs = cs & 0xffff;
    p->tss.ds = ds & 0xffff;
    p->tss.ss = ss & 0xffff;
    p->tss.fs = fs & 0xffff;
    p->tss.gs = gs & 0xffff;
    p->tss.ldt = _LDT(nr);
    p->tss.bitmap = 0x80000000;
    // 如果之前的进程使用了协处理器，这里复位TS标志
    if(last_task_used_math == current)
        __asm__ ("clts; fnsave %0"::"m" (p->tss.i387));
    // 复制进程页表
    if(copy_mem(nr, p)) {
        task[nr] = NULL;
        free_page((unsigned long)p);
        return -1;
    }

    // 设置好TSS和LDT描述符，然后将任务置位
    // 就绪状态，可以被OS调度
    set_tss_desc(gdt + (nr<<1) + FIRST_TSS_ENTRY, &(p->tss));
    set_ldt_desc(gdt + (nr<<1) + FIRST_LDT_ENTRY, &(p->ldt));
    p->state = TASK_RUNNING;

    return last_pid;    // 父进程返回儿子的pid
}

int find_empty_process(void) {
    int i;
    long tmp = last_pid;    // 记录最初起始进程号，用于标记循环结束
    
    while(1) {
        for(i = 0; i < NR_TASKS; i++) {
            if(task[i] && task[i]->pid == last_pid)
                break;
        }
        if(i == NR_TASKS) break;
        if((++last_pid) == tmp)
            break;
        // 判断last_pid 是否超出进程号数据表示范围，如果超出
        // 则置为1
        if(last_pid < 0) last_pid = 1; 
    }
    for(i = 1; i < NR_TASKS; i++)
        if(!task[i])
            return i;
    return -1;
}


