#################################################################
#                                                               #
#  Lesson 3: 将操作系统装载到正确的地址，设置GDT后              #
#  切换到保护模式                                               #
#                                                               #
#                                                               #
#################################################################

# 以下代码将操作系统完整的装载到内存中 Layout 为:
# 0x9000:0000 bootsect
# 0x9000:0200 setup
# 0x1000:0000 system
# 这段代码没有需要学生进行修改的地方
# 有兴趣的同学可以仔细读一下 read_it 子程序

	.code16        # 指定语法为 十六位汇编
	.equ SYSSIZE, 0x3000 # System Size in Clicks(1 click = 16bits)

# rewrite with AT&T syntax by falcon <wuzhangjin@gmail.com> at 081012
# Modified by VOID001<zhangjianqiu13@gmail.com> at 2017 03 05
# loads pretty fast by getting whole sectors at a time whenever possible.

	#.global _start, begtext, begdata, begbss, endtext, enddata, endbss
	.global _start # 程序开始处
	.global begtext, begdata, begbss, endtext, enddata, endbss
	.text
	begtext:
	.data
	begdata:
	.bss
	begbss:
	.text

	.equ SETUPLEN, 0x04			# Setup 程序占用的扇区数
	.equ BOOTSEG, 0x07c0		# 当此扇区被BIOS识别为启动扇区装载到内存中时，装载到0x07c0段处
	.equ INITSEG, 0x9000		# 我们的bootsector代码会被移动到这里
	.equ SETUPSEG, 0x9020		# setup.s的代码会被移动到这里(Bootsector 之后的一个扇区)
	.equ SYSSEG, 0x1000		    # system 程序的装载地址
								# 此时我们处于实模式(REAL MODE)中，对内存的寻址方式为
								# (段地址 << 4 + 偏移量) 可以寻址的线性空间为 20 位
	.equ ROOT_DEV, 0x301			# 指定/dev/fda为系统镜像所在的设备
	.equ ENDSEG, SYSSEG + SYSSIZE

	ljmp    $BOOTSEG, $_start   # 修改cs寄存器为BOOTSEG, 并跳转到_start处执行我们的代码

_start:
	mov $BOOTSEG, %ax			# 这里我们将启动扇区从0x07c0:0000处移动到0x9000:0000
	mov %ax, %ds				# rep mov 的用法如下
								# 源地址ds:si = 0x07c0:0000
								# 目的地址es:di = 0x9000:0000
								# 移动次数 %cx = 256
								# 因为是movsw所以每次移动一个Word(2Byte) 256 * 2 = 512 Byte	即为启动扇区的大小
	mov $INITSEG, %ax
	mov %ax, %es
	mov $256, %cx
	xor %si, %si
	xor %di, %di
	rep movsw					# 进行移动

	ljmp $INITSEG, $go			# 长跳转同时切换CS:IP
go:
	mov %cs,%ax				# 对DS, ES, SS寄存器进行初始化
	mov %ax, %ds
	mov %ax, %es			# 设置好 ES 寄存器，为后续输出字符串准备
	mov %ax, %ss			
	mov $0xFF00, %sp		# 设置好栈

load_setup:
	# 这里我们需要将软盘中的内容加载到内存中，并且跳转到相应地址执行代码
	mov $0x0000, %dx		# 选择磁盘号0，磁头号0进行读取
	mov $0x0002, %cx		# 从二号扇区，0轨道开始读(注意扇区是从1开始编号的)
	mov $INITSEG, %ax		# ES:BX 指向装载目的地址
	mov %ax, %es
	mov $0x0200, %bx		
	mov $02, %ah			# Service 2: Read Disk Sectors
	mov $4, %al				# 读取的扇区数
	int $0x13				# 调用BIOS中断读取
	jnc demo_load_ok		# 没有异常，加载成功
	mov $0x0000, %dx
	mov $0x0000, %ax		# Service 0: Reset the Disk
	int $0x13
	jmp load_setup			# 并一直重试，直到加载成功

demo_load_ok:				# Here will jump to where the demo program is
	#mov $SETUPSEG, %ax
	#mov %ax, %cs			# This is awful!! Do not change CS alone!! It will result in GDB cannot debug the code
							# And, of course the code will not work
	#mov %ax, %ds
	#ljmp $SETUPSEG, $0		# jump to where the demo program exists (Demo code, removed now)

	mov $0x00, %dl
	mov $0x0800, %ax
	int $0x13
	mov $0x00, %ch
	mov %cx, %cs:sectors+0
	mov $INITSEG, %ax
	mov %ax, %es

# 下面的程序用来显示一行信息
print_msg:
	mov	$0x03, %ah			# 在输出我们的信息前读取光标的位置, 会将光标当前所在行，列存储在DX里（DH为行, DL为列）
	xor	%bh, %bh
	int	$0x10
	
	mov	$20, %cx			# Set the output length
	mov	$0x0007, %bx		# page 0, color = 0x07 (from wikipedia https://en.wikipedia.org/wiki/INT_10H)
	mov $msg1, %bp
	mov	$0x1301, %ax		# write string, move cursor
	int	$0x10				# 使用这个中断0x10的时候，输出的内容是从 ES:BP 中取得的，因而要设置好 ES 和 BP


# 接下来将整个系统镜像装载到0x1000:0000开始的内存中
	mov $SYSSEG, %ax
	mov %ax, %es			# ES 作为参数
	call read_it
	call kill_motor

		
	mov %cs:root_dev,%ax
	cmp $0, %ax
	jne root_defined		# ROOT_DEV != 0, Defined root
	mov %cs:sectors+0, %bx    # else check for the root dev
	mov $0x0208, %ax
	cmp $15, %bx
	je	root_defined		# Sector = 15, 1.2Mb Floopy Driver
	mov $0x021c, %ax
	cmp $18, %bx			# Sector = 18 1.44Mb Floppy Driver
	je root_defined

undef_root:					# If no root found, loop forever
	jmp undef_root

root_defined:

	mov %ax, %cs:root_dev+0

# Now everything loaded into memory, we jump to the setup-routine
# which is now located at 0x9020:0000

	ljmp $SETUPSEG, $0


# Here is the read_it routine and kill_motor routine
# read_it 和 kill_motor 是两个子函数，用来快速读取软盘中的内容，以及关闭软驱
# 电机使用，下面是他们的代码

# 首先定义一些变量， 用于读取软盘信息使用

sread:  .word 1 + SETUPLEN		# 当前轨道读取的扇区数
head:   .word 0					# 当前读头
track:	.word 0					# 当前轨道


read_it:
	mov %es, %ax
	test $0x0fff, %ax
die:
	jne die				# If es is not at 64KB(0x1000) Boundary, then stop here
	xor %bx, %bx
rp_read:
	mov %es, %ax
	cmp $ENDSEG, %ax
	jb ok1_read			# If $ENDSEG > %ES, then continue reading, else just return
	ret
ok1_read:

	mov %cs:sectors+0, %ax
	sub sread, %ax
	mov %ax, %cx		# Calculate how much sectors left to read
	shl $9, %cx			# cx = cx * 512B
	add %bx, %cx		# current bytes read in now
	jnc ok2_read		# If not bigger than 64K, continue to ok_2
	je ok2_read
	xor %ax, %ax
	sub %bx, %ax
	shr $9, %ax
ok2_read:
	call read_track
	mov %ax, %cx		# cx = num of sectors read so far
	add sread, %ax

	cmp %cs:sectors+0, %ax
	jne ok3_read
	mov $1, %ax
	sub head, %ax
	jne ok4_read
	incw track
ok4_read:
	mov %ax, head
	xor %ax, %ax
ok3_read:
	mov %ax, sread
	shl $9, %cx
	add %cx, %bx		# HERE!!! I MADE A FAULT HERE!!!
	jnc rp_read			# If shorter than 64KB, then read the data again, else, adjust ES to next 64KB segment, then read again
	mov %es, %ax
	add $0x1000, %ax
	mov %ax, %es		# Change the Segment to next 64KB
	xor %bx, %bx
	jmp rp_read

# Comment for routine 0x13 service 2
# AH = 02
# AL = number of sectors to read	(1-128 dec.)
# CH = track/cylinder number  (0-1023 dec., see below)
# CL = sector number  (1-17 dec.)
# DH = head number  (0-15 dec.)
# DL = drive number (0=A:, 1=2nd floppy, 80h=drive 0, 81h=drive 1)
# ES:BX = pointer to buffer

read_track:				# This routine do the actual read
	push %ax
	push %bx
	push %cx
	push %dx
	mov track, %dx		# Set the track number $track, disk number 0
	mov sread, %cx
	inc %cx
	mov %dl, %ch
	mov head, %dx
	mov %dl, %dh
	mov $0, %dl
	and $0x0100, %dx
	mov $2, %ah
	int $0x13
	jc bad_rt
	pop %dx
	pop %cx
	pop %bx
	pop %ax
	ret

## Debug the load function
#d_load_setup:
#	# 这里我们需要将软盘中的内容加载到内存中，并且跳转到相应地址执行代码
#	mov $0x0000, %dx		# 选择磁盘号0，磁头号0进行读取
#	mov $0x0006, %cx		# 从二号扇区，0轨道开始读(注意扇区是从1开始编号的)
#	mov $SYSSEG, %ax		# ES:BX 指向装载目的地址
#	mov %ax, %es
#	mov $0x0200, %bx		
#	mov $02, %ah			# Service 2: Read Disk Sectors
#	mov $4, %al				# 读取的扇区数
#	int $0x13				# 调用BIOS中断读取
#ddemo: jnc ddemo		# 没有异常，加载成功
#	mov $0x0000, %dx
#	mov $0x0000, %ax		# Service 0: Reset the Disk
#	int $0x13
#	jmp d_load_setup			# 并一直重试，直到加载成功
## END DEBUG


bad_rt:
	mov $0, %ax
	mov $0, %dx
	int $0x13
	pop %dx
	pop %cx
	pop %bx
	pop %ax
	jmp read_track

kill_motor:
	push %dx
	mov $0x3f2, %dx
	mov $0, %al
	outsb
	pop %dx
	ret


sectors:
	.word 0


msg1:
	.byte 13,10
	.ascii "Hello VOID001!"
	.byte 13,10,13,10

	.= 508                    # 这里是对齐语法 等价于 .org 表示在该处补零，一直补到 地址为 510 的地方 (即第一扇区的最后两字节)
								# 然后在这里填充好0xaa55魔术值，BIOS会识别硬盘中第一扇区以0xaa55结尾的为启动扇区，于是BIOS会装载
								# 代码并且运行

root_dev:
	.word ROOT_DEV
boot_flag:
	.word 0xAA55

.text
	endtext:
.data
	enddata:
.bss
	endbss:
