###########################################################
#														  #
#		Lesson 4 设置IDT 对8259A进行编程    			  #
#													      #
###########################################################

.code16

.text

.equ SETUPSEG, 0x9020
.equ INITSEG, 0x9000
.equ SYSSEG, 0x1000
.equ LEN, 54

.global _start, begtext, begdata, begbss, endtext, enddata, endbss

.text
	begtext:
.data
	begdata:
.bss
	begbss:
.text

	
show_text:
	mov $SETUPSEG, %ax
	mov %ax, %es
	mov $0x03, %ah
	xor %bh, %bh
	int $0x10					# these two line read the cursor position
	mov $0x000a, %bx			# Set video parameter
	mov $0x1301, %ax
	mov $LEN, %cx
	mov $msg, %bp
	int $0x10

# 下面的代码调用BIOS中断将硬件的一些状态保存在0x9000:0000开始的内存处
# （注意这里会覆盖bootsect，不过无所谓，因为我们不再需要它了)
	ljmp $SETUPSEG, $_start
_start:

# 保存光标位置
# Comment for routine 10 service 3
# AH = 03
# BH = video page
# on return:
# CH = cursor starting scan line (low order 5 bits)
# CL = cursor ending scan line (low order 5 bits)
# DH = row
# DL = column
	mov $INITSEG, %ax
	mov %ax, %ds
	mov $0x03, %ah
	xor %bh, %bh
	int $0x10
	mov %dx, %ds:0
# 取扩展内存大小的值
# Comment for routine 0x15 service 0x88
# AH = 88h
# on return:
# CF = 80h for PC, PCjr
# = 86h for XT and Model 30
# = other machines, set for error, clear for success
# AX = number of contiguous 1k blocks of memory starting
# at address 1024k (100000h)
	mov $0x88, %ah
	int $0x15
	mov %ax, %ds:2

# 取显卡显示模式
# Comment for routine 10 service 0xf
# AH = 0F
# on return:
# AH = number of screen columns
# AL = mode currently set (see VIDEO MODES)
# BH = current display page
	mov $0x0f, %ah
	int $0x10
	mov %bx, %ds:4
	mov %ax, %ds:6

# 检查显示方式(EGA/VGA)并取参数
# Comment for routine 10 service 0x12
# We use bl 0x10
# BL = 10  return video configuration information
# on return:
# BH = 0 if color mode in effect
# = 1 if mono mode in effect
# BL = 0 if 64k EGA memory
# = 1 if 128k EGA memory
# = 2 if 192k EGA memory
# = 3 if 256k EGA memory
# CH = feature bits
# CL = switch settings
	mov $0x12, %ah
	mov $0x10, %bl
	int $0x10
	mov %ax, %ds:8
	mov %bx, %ds:10
	mov %cx, %ds:12

# 复制硬盘参数表信息
# 比较奇怪的是硬盘参数表存在中断向量里
# 第一个硬盘参数表的首地址在0x41中断向量处，第二个参数的首地址表在0x46中断向量处，紧跟着第一个参数表, 每个参数表长度为0x10 Byte

# 第一块硬盘参数表
	mov $0x0000, %ax
	mov %ax, %ds
	lds %ds:4*0x41, %si
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0080, %di
	mov $0x10, %cx
	rep movsb
# 第二块硬盘参数表
	mov $0x0000, %ax
	mov %ax, %ds
	lds %ds:4*0x46, %si
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0090, %di
	mov $0x10, %cx
	rep movsb

# 检查第二块硬盘是否存在，如果不存在的话就清空相应的参数表(
# Comment for routine 0x13 service 0x15
# AH = 15h
# DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
# on return:
# AH = 00 drive not present
# = 01 diskette, no change detection present
# = 02 diskette, change detection present
# = 03 fixed disk present
# CX:DX = number of fixed disk sectors; if 3 is returned in AH
# CF = 0 if successful
# = 1 if error
	mov $0x1500, %ax
	mov $0x81, %dl
	int $0x13
	jc no_disk1
	cmp $3, %ah
	je is_disk1
no_disk1:				# 没有第二块硬盘，那么就对第二个硬盘表清零，使用stosb
	mov $INITSEG, %ax
	mov %ax, %es
	mov $0x0090, %di
	mov $0x10, %cx
	mov $0x00, %ax
	rep stosb

is_disk1:

# 下面该切换到保护模式了～（终于要离开这个不安全，文档匮乏，调试费力的16bit实模式了）
# 进行切换保护模式的准备操作
	cli				# 关中断

# 我们先将system从0x1000:0000移动到0x0000:0000处
	mov $0x0000, %ax
	cld					# Direction = 0 move forward
do_move:
	mov %ax, %es
	add $0x1000, %ax
	cmp $0x9000, %ax	# Does we finish the move
	jz end_move
	mov %ax, %ds
	sub %di, %di
	sub %si, %si
	mov $0x8000, %cx	# Move 0x8000 word = 0x10000 Byte (64KB)
	rep movsw
	jmp do_move

# 下面我们加载 GDT, IDT 等
# 在这里补充加载GDT的代码，并在下面补充GDT表的结构

end_move:
	mov $SETUPSEG, %ax
	mov %ax, %ds
	lidt idt_48
	lgdt gdt_48	


# 开启A20地址线，使得可以访问64KB以上的内存
	inb $0x92, %al
	orb $0b00000010, %al
	outb %al, $0x92

# 这里我们会对8259A进行编程,很脏的活，不建议大家搞OwO(所以注释都是英文的辣)
	mov $0x11, %al			# Init ICW1, 0x11 is init command

	out %al, $0x20			# 0x20 is 8259A-1 Port
	.word 0x00eb, 0x00eb	# Time Delay jmp $+2, jmp $+2
	out %al, $0xA0			# And init 8259A-2
	.word 0x00eb, 0x00eb
	mov $0x20, %al			# Send Hardware start intterupt number(0x20)
	out %al, $0x21			# From 0x20 - 0x27
	.word 0x00eb, 0x00eb
	mov $0x28, %al
	out %al, $0xA1			# From 0x28 - 0x2F
	.word 0x00eb, 0x00eb
	mov $0x04, %al			# 8259A-1 Set to Master
	out %al, $0x21
	.word 0x00eb, 0x00eb
	mov $0x02, %al			# 8259A-2 Set to Slave
	out %al, $0xA1
	.word 0x00eb, 0x00eb
	mov $0x01, %al			# 8086 Mode
	out %al, $0x21
	.word 0x00eb, 0x00eb
	out %al, $0xA1
	.word 0x00eb, 0x00eb
	mov $0xFF, %al
	out %al, $0x21			# Mask all the interrupts now
	.word 0x00eb, 0x00eb
	out %al, $0xA1			


# 开启保护模式！
	mov %cr0, %eax
	bts $0, %eax		# Turn on Protect Enable (PE) bit
	mov %eax, %cr0

# Jump to protected mode
	.equ sel_cs0, 0x0008
	mov $0x10, %ax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	ljmp $sel_cs0, $0

# 请填写GDTR信息
gdt_48:					# This is the GDT Descriptor
	.word 0x800			# 
	.word 512+gdt, 0x9	# This give the GDT Base address 0x90200

idt_48:
	.word 0
	.word 0, 0

gdt:
	.word	0,0,0,0
	.word	0x07FF
	.word	0x0000
	.word	0x9A00
	.word	0x00C0

	.word	0x07FF
	.word	0x0000
	.word	0x9200
	.word	0x00C0

	

msg:
	.byte 13, 10
	.ascii "You've successfully load the floppy data into RAM"
	.byte 13, 10, 13, 10
