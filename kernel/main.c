/*
 * Default Routine to run after head.s
 *
 */

#include <linux/kernel.h>
#include <asm/system.h>

extern int video_x;
extern int video_y;

int memtest_main(void);

void main() {
    int ret;
    video_init();
    printk("Welcome to Linux0.11 Kernel Mode(NO)\n=w=\n");

    // 初始化物理页内存, 将 1MB - 16MB 地址空间的内存进行初始化
    mem_init(0x100000, 0x1000000);
    ret = mmtest_main();

    while(1);
}
