/*
 * Default Routine to run after head.s
 *
 */

#include <linux/kernel.h>

extern int video_x;
extern int video_y;

void main() {
    int i;
    video_init();
    printk("Welcome to Linux0.11 Kernel Mode(NO)\n");
    printk(" =w= \n");
    
    while(1);
}
