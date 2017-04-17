// 从Port读入数据到al, _v变量的值就是al的值
#define inb(port) ({\
unsigned char _v; \
__asm__ volatile("inb %%dx, %%al":"=a" (_v):"d" (port)); \
_v; \
    })

#define outb(port, value) \
    __asm__ ("outb %%al, %%dx"::"a" (value), "d" (port));

// 带有延时的outb
#define outb_p(port, value) \
    __asm__ volatile("outb %%al, %%dx\n\t" \
            "jmp 1f\n\t" \
            "1: jmp 1f\n\t" \
            "1:"::"a" (value), "d" (port))

#define inb_p(port) ({ \
unsigned char _v; \
__asm__ volatile("inb %%dx, %%al\n\t" \
        "jmp 1f\n\t" \
        "1: jmp 1f\n\t" \
        "1:": "=a" (_v): "d" (port)); \
_v; \
        })
