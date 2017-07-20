include Makefile.header
#AS=i686-elf-as
#LD=i686-elf-ld
#OBJCOPY=i686-elf-objcopy
#QEMU=qemu-system-i386
#BOCHS=bochs
#STRIP=i686-elf-strip

.PHONY=clean run all boot/head.o boot/bootsect boot/setup kernel/kernel.o

all: bootimg

OBJS = boot/head.o init/main.o kernel/kernel.o mm/mm.o lib/lib.o
DRIVERS = kernel/chr_drv/chr_drv.a

system: $(OBJS) $(DRIVERS)
	@$(LD) -T ldS.ld $(OBJS) $(DRIVERS) -o system.sym
	@$(STRIP) system.sym -o system.o
	@$(OBJCOPY) -O binary -R .note -R .comment system.o system

kernel/chr_drv/chr_drv.a:
	@make -C kernel/chr_drv/

kernel/kernel.o:
	@make -C kernel

boot/head.o:
	@make head.o -C boot

boot/bootsect:
	@make bootsect -C boot

boot/setup:
	@make setup -C boot

init/main.o:
	@make main.o -C init

lib/lib.o:
	@make lib.o -C lib

mm/mm.o:
	@make -C mm

# Squash the bootimg together
bootimg: boot/setup boot/bootsect system
	@echo -e "\e[1;33mStart building NEU-OS image..."
	@dd if=boot/bootsect of=bootimg bs=512 count=1 2>/dev/null
	@dd if=boot/setup of=bootimg bs=512 count=4 seek=1 2>/dev/null
	@dd if=system of=bootimg bs=512 seek=5 2>/dev/null
	@echo -e "\e[1;0;32mBuild bootimg done"

run: bootimg
	$(QEMU) -boot a -fda bootimg -serial stdio

run_bochs: bootimg
	$(BOCHS) -q

run_debug:
	$(QEMU) -boot a -fda bootimg -S -s

disassemble: system.sym
	objdump -S system.sym | less

clean:
	@rm -f bootsect *.o setup *.sym bootimg a.out binary head  system
	@make clean -C boot
	@make clean -C kernel
	@make clean -C mm
	@make clean -C init
	@make clean -C lib
