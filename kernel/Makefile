include ../Makefile.header

OBJS = printk.o panic.o traps.o asm.o sched.o system_call.o sys.o fork.o \
	   serial_debug.o signal.o signal_demo.o exit.o libc_restore.o

LDFLAGS	+= -r
CFLAGS += -I../include

.PHONY=clean run all

all: kernel.o chr_drv/chr_drv.a

kernel.o: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.o $(OBJS)

chr_drv/chr_drv.a:
	@make -C chr_drv

.c.o: 
	@$(CC) $(CFLAGS) \
		-c -o $*.o $<

clean:
	- rm -f $(OBJS) kernel.o
	- @make clean -C chr_drv
