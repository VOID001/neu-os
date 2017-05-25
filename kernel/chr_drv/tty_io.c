#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/sched.h>
#include <asm/system.h>
#include <asm/io.h>
#include <serial_debug.h>

extern void keyboard_interrupt(char scancode);

void con_init(void) {
    register unsigned char a;
    set_trap_gate(0x21, &keyboard_interrupt);
    outb_p(0x21, inb_p(0x21)&0xfd);
    a = inb_p(0x61);
    outb_p(0x61, a | 0x80);
    outb(0x61, a);
}

void tty_init(void) {
    con_init();
}
