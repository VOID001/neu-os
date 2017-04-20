/*
 * Default Routine to run after head.s
 *
 */

#include <linux/kernel.h>
#include <asm/system.h>


int memtest_main(void);

void main() {
    int ret;
    video_init();
    trap_init();
    sched_init();
    mem_init(0x100000, 0x300000);

    // 初始化物理页内存, 将 1MB - 16MB 地址空间的内存进行初始化
    sti();
    //ret = mmtest_main();

    while(1);
}
