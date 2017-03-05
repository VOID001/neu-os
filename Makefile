AS=i686-elf-as
LD=i686-elf-ld
OBJCOPY=i686-elf-objcopy
QEMU=qemu-system-i386

LDFLAGS	+= -Ttext 0

.PHONY=clean run all

all: bootsect

bootsect: bootsect.s
	@$(AS) -o bootsect.o bootsect.s
	@$(LD) $(LDFLAGS) -o bootsect bootsect.o
	@cp -f bootsect bootsect.sym
	@$(OBJCOPY) -R .pdr -R .comment -R.note -S -O binary bootsect

run: bootsect
	$(QEMU) -boot a -fda bootsect

clean:
	@rm -f bootsect bootsect.o bootsect.sym
