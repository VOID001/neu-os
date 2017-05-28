/*
 * console low level implementation
 */

#include <linux/kernel.h>
#include <linux/head.h>
#include <linux/mm.h>
#include <linux/tty.h>
#include <asm/system.h>
#include <asm/io.h>
#include <stdarg.h>
#include <stddef.h>
#include <serial_debug.h>

#define PAGE_SIZE 4096
#define VIDEO_MEM 0xB8000
#define VIDEO_X_SZ 80
#define VIDEO_Y_SZ 25
#define TAB_LEN 8
#define CALC_MEM(x, y) (2*((x) + 80*(y)))

extern void keyboard_interrupt(char scancode);

int video_x, video_y;

long user_stack[PAGE_SIZE>>2];

struct video_info {
    unsigned int retval;        // Return value
    unsigned int colormode;     // Color bits
    unsigned int feature;       // Feature settings
};

extern int video_x;
extern int video_y;

char *video_buffer = (char *)VIDEO_MEM;

void video_init() {
    // struct video_info *info = (struct video_info *)0x9000;

    video_x = 0;
    video_y = 0;
    video_clear();
    update_cursor(video_y, video_x);
}

int video_getx() {
    return video_x;
}

int video_gety() {
    return video_y;
}

void update_cursor(int row, int col) {
    unsigned int pos = ((unsigned int)row * VIDEO_X_SZ) + (unsigned int)col;
    // LOW Cursor port to VGA Index Register
    outb(0x3D4, 0x0F);
    outb(0x3D5, (unsigned char)(pos & 0xFF));
    // High Cursor port to VGA Index Register
    outb(0x3D4, 0x0E);
    outb(0x3D5, (unsigned char)((pos >> 8) & 0xFF));
    return ;
}

int get_cursor() {
    int offset;
    outb(0x3D4,0xF);
    offset=inb(0x3D5)<<8;
    outb(0x3D4,0xE);
    offset+=inb(0x3D5);
    return offset;
}

void video_putchar(char ch) {
    if(ch == '\n') {
        video_x = 0;
        video_y++;
    }
    else if(ch == '\t') {
        while(video_x % TAB_LEN) video_x++;
    }
    else if(ch == '\b') {
        video_x--;
        if (video_x < 0) {
            video_x = VIDEO_X_SZ;
            video_y--;
            if (video_y < 0) video_y = 0;
        }
        // erase char
        video_putchar_at(' ', video_x, video_y, 0x0F);
    }
    else {
        video_putchar_at(ch, video_x, video_y, 0x0F);
        video_x++;

    }
    if(video_x >= VIDEO_X_SZ) {
        video_x = 0;
        video_y++;
    }
    if(video_y >= VIDEO_Y_SZ) {
        roll_screen();
        video_x = 0;
        video_y = VIDEO_Y_SZ - 1;
    }

    update_cursor(video_y, video_x);
    return ;
}

void con_write(struct tty_struct *tty) {
    char ch;
    s_printk("Console write called\n");
    while (!tty_isempty_q(&tty->write_q)) {
        tty_queue_stat(&tty->write_q);
        ch = tty_pop_q(&tty->write_q);
        video_putchar(ch);
    }
    s_printk("Console write returned\n");
    return ;
}

void video_clear() {
    int i;
    int j;
    video_x = 0;
    video_y = 0;
    for(i = 0; i < VIDEO_X_SZ; i++) {
        for(j = 0; j < VIDEO_Y_SZ; j++) {
           video_putchar_at(' ', i, j, 0x0F);  // DO NOT USE 0x00 HERE, YOU WILL LOSE YOUR LOVELY BLINKING CURSOR(
        }
    }
    return ;
}

void video_putchar_at(char ch, int x, int y, char attr) {
    if(x >= 80)
        x = 80;
    if(y >= 25)
        y = 25;
    *(video_buffer + 2*(x+80*y)) = ch;              // You should write it correct, think carefully
    *(video_buffer + 2*(x+80*y)+1) = attr;          // Previous code : (video_buffer + 2*x + 80*y) (suck)
    return ;
}

void roll_screen() {
    int i;
    // Copy line A + 1 to line A
    for(i = 1; i < VIDEO_Y_SZ; i++) {
        memcpy(video_buffer + (i - 1) * 80 * 2, video_buffer + i * 80 * 2, VIDEO_X_SZ, 2*sizeof(char));
    }
    // Clear the last line
    for(i = 0; i < VIDEO_X_SZ; i++) {
        video_putchar_at(' ', i, VIDEO_Y_SZ - 1, 0x0F);
    }
    return ;
}

void memcpy(char *dest, char *src, int count, int size) {
    int i;
    int j;
    for(i = 0; i < count; i++) {
        for(j = 0; j < size; j++) {
            *(dest + i*size + j) = *(src + i*size + j);
        }
    }
    return ;
}

void con_init(void) {
    register unsigned char a;
    set_trap_gate(0x21, &keyboard_interrupt);
    video_init();
    outb_p(0x21, inb_p(0x21)&0xfd);
    a = inb_p(0x61);
    outb_p(0x61, a | 0x80);
    outb(0x61, a);
}
