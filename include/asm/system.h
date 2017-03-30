#define sti()  __asm__ volatile ("sti"::)
#define cli()  __asm__ volatile ("cli"::)
#define nop()  __asm__ volatile ("nop"::)
#define iret() __asm__ volatile ("iret"::)
