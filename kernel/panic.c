/*
 * Use for trigger a kernel panic
 * When panic, it will print out a message and then 
 * dead, use for alerting a major problem
 */
#define PANIC

#include <linux/kernel.h>

void panic(const char *str) {
    printk("Kernel Panic: %s\n", str);
    for(;;);
}

