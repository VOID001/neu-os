.code16

.text

.equ SETUPSEG, 0x9020
.equ INITSEG, 0x9000
.equ LEN, 54

show_text:
	mov $SETUPSEG, %ax
	mov %ax, %es
	mov $0x03, %ah
	xor %bh, %bh
	int $0x10

	mov $0x000a, %bx
	mov $0x1301, %ax
	mov $LEN, %cx
	mov $msg, %bp
	int $0x10

	ljmp $SETUPSEG, $_start

_start:
# Save cursor position
	mov $INITSEG, %ax
	mov %ax, %ds
	mov $0x03, %ah
	xor %bh, %bh
	int $10
	mov %dx, %ds:0

# Extened memory store
# 0x15 0x88
	mov $0x88, %ah
	int $0x15
	mov %ax, %ds:2

# VGA Display Mode
	mov $0x12, %ah
	mov $0x10, %bl
	int $0x10
	mov %ax, %ds:8
	mov %bx, %ds:10
	mov %cx, %ds:12

# Harddisk Parameter Table
	mov $0x0000, %ax
	mov %ax, %ds
	lds %ds:4*0x41, %si
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0080, %di
	mov $0x10, %cx
	rep movsb
	
	mov $0x0000, %ax
	mov %ax, %ds
	lds %ds:4*0x46, %si
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0090, %di
	mov $0x10, %cx
	rep movsb

# Check second harddisk parameter
	mov $0x1500, %ax
	mov $0x81, %dl
	int $0x13
	jc no_disk1
	cmp $3, %ah
	je is_disk1

no_disk1:
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0090, %di
	mov $0x10, %cx
	mov $0x00, %ax
	rep stosb

is_disk1:
	# Prepare to enter protect mode
	cli

	# Move System image to 0x0000:0x0000
	mov $0x0000, %ax
do_move:
# rep movsw ds:si -> es:di
# 0x1000:0 -> 0x0000:0
# 0x2000:0 -> 0x1000:0
	mov %ax, %es
	add $0x1000, %ax
	cmp $0x9000, %ax
	jz end_move
	mov %ax, %ds
	sub %di, %di
	sub %si, %si
	mov $0x8000, %cx
	rep movsw
	jmp do_move

# GDT & IDT load
end_move:
	mov $SETUPSEG, %ax
	mov %ax, %ds
	lgdt gdt_48
	lidt idt_48

enable_a20:
	in $0x92, %al
	or $0x02, %al
	out %al, $0x92

# CR0 Protect Enabled = 1

	mov %cr0, %eax
	bts $0, %eax
	mov %eax, %cr0

# Jump to Protect Mode
# Long Jump

# <Segment Selector>:<Offset>
	.equ sel_cs0, 0x0008
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	ljmp $sel_cs0, $0	

# 8259A Programming



# GDT Descriptor
gdt_48:
	.word 0x800
	.word 512+gdt, 0x9

# Global Descriptor Table

gdt:
	.word 0, 0, 0, 0
# Code Segement Size 8MB BaseAddr:0x0000:0x0000
	.word 0x07FF	# Limit 0-2047
	.word 0x0000	# Base
	.word 0x9A00	# 1 00 1 1010 0000 0000
	.word 0x00C0	# 0000 0000 1 1 0 0 0000
# Data Segment
	.word 0x07FF	# Limit 0-2047
	.word 0x0000	# Base
	.word 0x9200	# 1 00 1 0010 0000 0000
	.word 0x00C0	# 0000 0000 1 1 0 0 0000

idt_48:
	.word 0
	.word 0, 0

idt:


msg:
	.byte 13, 10
	.ascii "You've sucessfully load the floppy data into RAM"
	.byte 13, 10, 13, 10
	

#Proof Floppy Disk Load OK
