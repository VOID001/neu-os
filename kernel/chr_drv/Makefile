# Makfile for char dev driver
include ../../Makefile.header

CFLAGS += -I../../include
CPP += -I../../include

OBJS = console.o tty_io.o keyboard.o serial.o rs_io.o \
	   tty_ioctl.o

all: chr_drv.a

keyboard.s: kb.S
	@$(CPP) kb.S -o keyboard.s

.c.s:
	@$(CC) $(CFLAGS) \
		-S -o $*.s $<
.c.o:
	@$(CC) $(CFLAGS) \
		-c -o $*.o $<

.s.o:
	@$(AS) -o $*.o $<


chr_drv.a: $(OBJS)
	@$(AR) rcs chr_drv.a $(OBJS)
	@sync

clean: 
	- rm *.o *.a $(OBJS) keyboard.s
