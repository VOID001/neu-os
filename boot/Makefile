include ../Makefile.header

#AS=i686-elf-as
#LD=i686-elf-ld
#OBJCOPY=i686-elf-objcopy
#QEMU=qemu-system-i386
#BOCHS=bochs

LDFLAGS	+= -Ttext 0

.PHONY=clean run all

all: bootsect setup head.o

# Bootsector is RAW Binary
# So is setup program

bootsect: bootsect.s
	@$(AS) -n -g -o bootsect.o bootsect.s
	@$(LD) $(LDFLAGS) -o bootsect bootsect.o
	@cp -f bootsect bootsect.sym
	@$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary bootsect

setup: setup.s
	@$(AS) -n -g -o setup.o setup.s
	@$(LD) $(LDFLAGS) -o setup setup.o
	@cp -f setup setup.sym
	@$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary setup

head.o: head.s
	@$(AS) -n -g -o head.o head.s

	#@cp -f head head.sym
	#@$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary head
	#@$(LD) $(LDFLAGS) -o head head.o

binary: binary.s
	@$(AS) -n -g -o binary.o binary.s
	@$(LD) $(LDFLAGS) -o binary binary.o
	@cp -f binary binary.sym
	@$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary binary

#bootimg: setup bootsect head binary
#	@dd if=bootsect of=bootimg bs=512 count=1
#	@dd if=setup of=bootimg bs=512 count=4 seek=1
#	@dd if=binary of=bootimg bs=512 seek=5
#	@echo "Build bootimg done"

run: bootimg
	$(QEMU) -boot a -fda bootimg

run_bochs: bootimg
	$(BOCHS) -q

run_debug:
	$(QEMU) -boot a -fda bootimg -S -s

clean:
	@rm -f bootsect *.o setup *.sym bootimg a.out binary head 
