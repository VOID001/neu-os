#define inb(port) ({\
unsigned char _v; \
__asm__ volatile("inb %%dx, %%al":"=a" (_v):"d" (port)); \
_v; \
    })

#define outb(port, value) \
    __asm__ ("outb %%al, %%dx"::"a" (value), "d" (port));
