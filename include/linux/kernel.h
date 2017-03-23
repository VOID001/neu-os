#ifndef _KERNEL_H
#define _KERNEL_H
 
void printk(char *fmt, ...);     // Simplest printk function to use
void video_putchar_at(char ch, int x, int y, char attr);
void video_putchar(char ch);
void video_clear();
void video_init();
int video_getx();
int video_gety();
void roll_screen();
void memcpy(char *dest, char *src, int count, int size);
void printnum(int num, int base, int sign);
int get_cursor();
void update_cursor(int row, int col);

int video_x, video_y;

#endif
