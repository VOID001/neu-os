/*
 * Routine use to handle physical page memory
 * memory.c 涉及到的函数均为物理页内存,以及页表管理函数
 * 并且实现了 Demand Loading 以及 Copy On Write
 */

// 以下是一些在 head.s 里已经规划好的参数,在这里以宏的形式再次给出

#include <linux/kernel.h>
#include <linux/head.h>
#include <serial_debug.h>

#define LOW_MEM 0x100000ul // 0x00000000 - 0x00100000 为物理内存低 1MB 空间, 是系统代码所在
#define PAGING_MEMORY (15*1024*1024) // 剩余 15MB 空闲物理内存用于分页
#define PAGING_PAGES (PAGING_MEMORY >> 12) // 分页后的页数
#define MAP_NR(addr) (((addr) - LOW_MEM) >> 12) // 计算当前物理地址的对应页号
#define USED 100 // 占用状态, 为什么是100呢, 我不是很理解

// 该宏用于复制一页物理内存从 from 到 to
#define copy_page(from, to) \
    __asm__ volatile ("cld; rep; movsl;":"S" (from), "D" (to), "c" (1024))

// 用于使TLB失效, 刷新缓存
#define invalidate() \
    __asm__ volatile("mov %%eax, %%cr3"::"a" (0))

static unsigned long HIGH_MEMORY = 0;
void un_wp_page(unsigned long * table_entry);

// 最基本的, linux0.11 使用内存字节位图来管理物理页的状态, 声明时全部填零
// 每一个字节表示这个物理页面被共享次数
static unsigned char mem_map[ PAGING_PAGES ] = {0,};

// 在当前状态因为没有进程管理, 如果内存超限直接panic
static inline void oom() {
    panic("Out of Memory!! QWQ\n");
}

// 此函数对物理页映像进行初始化(mem_map)，将内核使用的部分内存映像初始设置为占用
// 将从start_mem -> end_mem 处的内存初始化为没有被使用 ( 0 - 1MB 空间不在考虑之内)
// >>12 = /4KB(一页的大小)
//
void mem_init(unsigned long start_mem, unsigned long end_mem) {
    unsigned long i;
    HIGH_MEMORY = end_mem;      // 物理内存最高处为 end_mem
    for(i = 0; i < PAGING_PAGES; i++)
        mem_map[i] = USED;      // 除了start_mem ~ end_mem 的区域物理页都应该为被占用状态

    i = (unsigned long)MAP_NR(start_mem);
    end_mem -= start_mem;
    end_mem >>= 12;              // 计算多少页需要设置为free
    while(end_mem-->0) {
        mem_map[i++] = 0;
    }
    return ;
}

// 仅仅用来显示当前 memory 用量的一个小函数
void calc_mem(void) {
    int i, j, k, free = 0;
    long *pg_tbl;

    for(i = 0; i < PAGING_PAGES; i++)
        if(!mem_map[i]) free++;
    printk("%d pages free (of %d in total)\n", free, PAGING_PAGES);
    
    // 遍历除了页表页目录的其余页表项, 如果页面有效, 则统计有效页面数量
    for(i = 2; i < 1024; i++) {
        if(pg_dir[i] & 1) {     // 先检查 Dir 是否存在
            pg_tbl = (long *)(0xfffff000 & pg_dir[i]);  // 计算 pg_tbl 的地址
            for(j = k = 0; j < 1024; j++) {
                if(pg_tbl[j] & 1) {     // 检查 Entry 是否存在
                    k++;
                }
            }
            printk("PageDir[%d] uses %d pages\n", i, k);
        } 
    }
    return ;
}

// 此函数用于获取第一个（按照顺序来说是最后一个)空闲的内存物理页，
// 这个函数从mem_map的末尾开始遍历，直到遇到某个物理页映像为没有被占用状态，这时计算出该页的物理地址
// 初始化该页内容为0, 并返回页起始地址
// 如果不存在可以用的页则返回 0
unsigned long get_free_page(void) {
   register unsigned long __res asm("ax");

   __asm__ volatile ("std; repne; scasb\n\t"
                "jne 1f\n\t"
                "movb $1, 1(%%edi)\n\t"
                "sall $12, %%ecx\n\t"
                "addl %2, %%ecx\n\t"
                "movl %%ecx, %%edx\n\t"
                "movl $1024, %%ecx\n\t"
                "leal 4092(%%edx), %%edi\n\t"
                "rep; stosl;\n\t"
                "movl %%edx, %%eax\n"
                "1: cld"
                : "=a" (__res)
                : "0" (0), "i" (LOW_MEM), "c" (PAGING_PAGES), "D" (mem_map + PAGING_PAGES - 1));    // 从尾端开始检查是否有可用的物理页
   return __res;
}

// 释放一页物理页
// 就是将mem_map中相应的byte置0,以及做一些必要的error_check
// 包括是否访问了内核内存,还有是否超出了物理内存(16MB)边界
void free_page(unsigned long addr) {
    if(addr < LOW_MEM) return ;      // 决不允许操作物理内存低端 
    if(addr >= HIGH_MEMORY) return ; // 也不能超过可用内存高端

    addr = MAP_NR(addr);        // 计算出需要的页号
    if(mem_map[addr]--) return;     // 如果该页为被使用状态那么减少引用计数并返回
    mem_map[addr] = 0;
    panic("Tring to free free page!");  // 不应该出现这种情况 因而引发panic
}

// 释放页表, 以及相应对应的物理页释放,并释放该页目录项占用的物理页
int free_page_tables(unsigned long from, unsigned long size) {
    unsigned long *pg_tbl;
    unsigned long *pg_dir, nr;

    // 检查是否为 4MB 内存边界(该函数仅仅处理连续的4MB内存块)
    if(from & 0x3fffff)
        panic("free_page_tables called with wrong alignment");
    // 如果要释放低 4MB 内存,也会引发错误(也就是from == 0)
    if(!from)
        panic("try to free up swapper memory space");
    size = (size + 0x3fffff) >> 22;             // 计算占的目录项数
    // 计算目录项地址
    pg_dir = (unsigned long *)((from >> 20) & 0xffc);
    // 下面开始释放, 首先释放页表项, 后释放页目录项
    // 注意释放的是连续的目录/页表项, 因而 pg_dir 自增即可
    for(; size-->0; pg_dir++) {
         if(!(*pg_dir & 1)) // 说明该目录项没有被使用
             continue;
         pg_tbl = (unsigned long *)(*pg_dir & 0xfffff000);      // 从页目录项中取出页表项地址
         for(nr = 0; nr < 1024; nr++) { // 开始释放页表项
             if(*pg_tbl & 1)        // 此页存在(P = 1)
                 free_page(0xfffff000 & *pg_tbl);   // 释放此页
             *pg_tbl = 0;
             pg_tbl++;
         }
         // 然后释放掉这个页目录所占的页
         free_page(0xfffff000 & *pg_dir);
         *pg_dir = 0;
    }
    invalidate();
    return 0;
}

// 用来将一页内存放入页表(和页目录), 即将相应的页表,页目录项填充,
// 如果要put的物理页在内存中还是unset状态(mem_map[xxx] = 0),那么先去获取一个可用的物理页
// 此函数假定了 pg_dir = 0 (hardcode)
// 成功时返回该页的物理地址, 失败/OOM 时返回0
// 参数 page 为页面的物理地址 address 为线性地址
unsigned long put_page(unsigned long page, unsigned long address) {
    unsigned long *pg_tbl, tmp;
    
    if(page < LOW_MEM || page >= (unsigned long)HIGH_MEMORY)
        printk("Trying to put page %x at %x\n", page, address);
    // mem_map 中此页为unset状态
    if(mem_map[MAP_NR(page)] != 1)
        printk("mem_map disagrees with %x at %x\n", page, address);
    // 计算该线性地址对应的页目录地址
    pg_tbl = (unsigned long *) ((address >> 20) & 0xffc);
    // 如果页目录存在则直接取出 pg_tbl
    // printk("Params: pg_tbl = %x, entry = %x\n", pg_tbl, (address >> 12) & 0x3ff);
    if(*pg_tbl & 1) {
        // printk("Page table now available\n");
        pg_tbl = (unsigned long *) (*pg_tbl & 0xfffff000);
    }
    // 否则申请物理页放置页目录
    else {
        if(!(tmp = get_free_page())) {
            printk("NO FREE PAGE!");
            return 0;
        }
        *pg_tbl = tmp | 7;
        // printk("Tmp = %x\n", tmp);
        // printk("Page Table = %x\n", *pg_tbl);
        pg_tbl = (unsigned long *) tmp;
    }
    // printk("Put Page Success\n");
    pg_tbl[(address >> 12) & 0x3ff] = page | 7;
    //invalidate();
    return page;
}

// 为线性地址 address 申请一个空物理页
// 失败则返回异常
void get_empty_page(unsigned long address) {
    unsigned long tmp;
    if(!(tmp = get_free_page()) || !put_page(tmp, address)) {
        free_page(tmp);
        oom();
    }
    return ;
}

// 写页面验证, 即验证目前要操作的页面(线性地址)是否合法以及可以被写入
// 如果页面存在但是不可写入, 则会尝试解除页面写入保护 (un_wp_page())
void write_verify(unsigned long address) {
    unsigned long page;
    // 检查页目录项是否存在
    if(!( (page = *((unsigned long *)((address >> 20) & 0xffc))) & 1)) {
        return ;
    }
    // 取页表首地址
    page = page & 0xfffff000;
    page += ((address >> 10) & 0xffc);
    if((*(unsigned long *)page & 3) == 1) { // 页表P = 1, R/W = 0
        un_wp_page((unsigned long *)page);
    }
    return ;
}

// 解除页面的写入保护
// 解除write protect,具体两个情况, 如果这个页面在 1MB 以上空间
// 并且存在,那么就直接在FLAG上添加 W FLAG 并且刷新TLB, 否则的话申请一个新的页面
// 并复制老页面的内容到新页面(Copy On Write)
//
void un_wp_page(unsigned long * table_entry) {
    unsigned long old_page, new_page;
    old_page = *table_entry & 0xfffff000;
    // 页面存在且位于1MB以上
    if(old_page >= LOW_MEM && mem_map[MAP_NR(old_page)] == 1) {
        *table_entry |= 2;
        invalidate();
        return ;
    }
    // 无法分配新的页面
    if(!(new_page = (unsigned long)get_free_page))
        oom();
    // 页面被共享, 因为要进行 COW 之后该页面就是独立的了,所以引用计数 -1
    if(old_page >= LOW_MEM)
        mem_map[MAP_NR(old_page)]--;
    *table_entry = new_page | 7;
    invalidate();
    return ;
}

// 缺页异常会调用此函数
void do_wp_page(unsigned long error_code, unsigned long address) {
    error_code = error_code; // 纯粹为了消除警告
    un_wp_page((unsigned long *) ((((address >> 10) & 0xffc) +
                ((*(unsigned long *)((address >> 20) & 0xffc)))) & 0xfffff000));
}

// 缺页异常会调用此函数
// 缺页异常会调用的主要处理函数, (有进程管理相关代码, 暂时略),  如果当前进程executable是空的
// 或者地址已超出进程数据,则尝试取一个空页面并返回, 如果失败
// 则尝试共享页面,如果还失败,则尝试取一个新的页面(不在内存中的),再失败则报错OOM
// 参数 error_code, 错误号; address 线性地址
void do_no_page(unsigned long error_code, unsigned long address) {
    //unsigned long tmp;
    unsigned long page;

    s_printk("Page Fault at [%x], errono %d\n", address, error_code);
    address &= 0xfffff000;
    if(!(page = get_free_page()))
        oom();
    if(put_page(page, address))
        return ;
    free_page(page);
    oom();
}

// 下面的两个函数是多进程需要使用到的，copy_page_tables 和其姊妹函数
// free_page_tables

// copy_page_table 用来拷贝线性地址连续一页的空间到另一个线性地址，
// 做法为仅仅拷贝其页表项内容，不拷贝其物理内存内容
// 同时当from = 0时表示从内核拷贝，这时我们不需要拷贝 4MB ,仅仅拷贝 640 KB
int copy_page_tables(unsigned long from, unsigned long to, unsigned long size) {
    unsigned long *from_page_table;
    unsigned long *to_page_table;
    unsigned long this_page;
    unsigned long *from_dir, *to_dir;
    unsigned long nr;

    // 检查内存边界，必须是4MB的整数倍，否则 panic 
    if((from & 0x3fffff) || (to & 0x3fffff)) {
        panic("copy_page_tables called with wrong alignment");
    }
    // 获取线性地址对应的页目录项
    from_dir = 0 + (unsigned long *)((from >> 20) & 0xffc);
    to_dir = 0 + (unsigned long *)((to >> 20) & 0xffc);
    size = ((unsigned) (size + 0x3fffff)) >> 22;

    for(; size-->0; from_dir++, to_dir++) {
        if(1 & *to_dir)
            panic("copy_page_tables: already exists");
        // from_dir 不存在，就跳过这页页表的复制
        if(!(1 & *from_dir))
            continue;
        if(!(to_page_table = (unsigned long *) get_free_page()))
            return -1;
        *to_dir = ((unsigned long)to_page_table | 7);
        // 判断是不是复制内核页，如果是则只copy 640KB(0xA0=160个页面)
        nr = (from == 0)?0xA0:1024;
        for(; nr-->0; from_page_table++, to_page_table++) {
            this_page = *from_page_table;
            if(!(1 & this_page))
                continue;
            // 这里将复制的页设置为只读
            this_page &= (unsigned long)~2;
            *to_page_table = this_page;
            // 如果复制的页面地址为高于LOW_MEM，说明不是从内核空间复制
            // 同时把原页面也设置为只读，这样二者共享了物理页面，如果向任何一个页面写入都会
            // 引发页保护异常（Page Fault），触发写时复制(Copy On Write COW)
            if(this_page > LOW_MEM) {
                *from_page_table = this_page;
                mem_map[MAP_NR(this_page)]++;
            }
        }
    }
    invalidate();
    return 0;
}
