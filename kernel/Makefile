CC=i686-elf-gcc
AS=i686-elf-as
LD=i686-elf-ld
OBJCOPY=i686-elf-objcopy
QEMU=qemu-system-i386
BOCHS=bochs
STRIP=i686-elf-strip

OBJS = printk.o main.o panic.o

LDFLAGS	+= -r
CFLAGS += -I../include

.PHONY=clean run all

all: kernel.o

kernel.o: $(OBJS)
	$(LD) $(LDFLAGS) -o kernel.o $(OBJS)

.c.o: 
	@$(CC) $(CFLAGS) \
		-c -o $*.o $<

clean:
	rm -f $(OBJS) kernel.o
