all: Image

.PHONY=clean run-qemu

run-qemu: bootsect
	@qemu-system-i386 -boot a -fda bootsect

bootsect.o:
	- as --32 bootsect.S -o bootsect.o

bootsect: bootsect.o ld-bootsect.ld
	- ld -T ld-bootsect.ld bootsect.o -o bootsect
	- objcopy -O binary -j .text bootsect

clean:
	- rm -f *.o bootsect
