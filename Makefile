all: Image

.PHONY=clean run-qemu

run-qemu: Image
	- @qemu-system-i386 -boot a -fda Image

bootsect.o: bootsect.S
	- @as --32 bootsect.S -o bootsect.o

bootsect: bootsect.o ld-bootsect.ld
	- @ld -T ld-bootsect.ld bootsect.o -o bootsect
	- @objcopy -O binary -j .text bootsect

Image:	bootsect demo
	- @dd if=bootsect of=Image bs=512 count=1
	- @dd if=demo of=Image bs=512 count=4 seek=1
	- @echo "Image built done"

demo: demo.o
	- @ld -T ld-bootsect.ld demo.o -o demo
	- @objcopy -O binary -j .text demo

demo.o: demo.S
	- @as --32 demo.S -o demo.o

clean:
	- @rm -f *.o bootsect
