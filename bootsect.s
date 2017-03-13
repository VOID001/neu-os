#################################################################
#                                                               #
#  Lesson 2: 加载软盘中的内容到内存，并执行相应代码				#
#  Goal: 了解 int 0x13 的使用方法，以及了解如何使用ljmp切换     #
#  CS 和IP														#
#                                                               #
#################################################################

	.code16        # 指定语法为 十六位汇编
	.equ SYSSIZE, 0x3000 # System Size in Clicks

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

	.equ BOOTSEG, 0x07c0		# 当此扇区被BIOS识别为启动扇区装载到内存中时，装载到0x07c0段处
	.equ INITSEG, 0x9000
	.equ DEMOSEG, 0x1000		# 要调用的函数在的段地址
								# 此时我们处于实模式(REAL MODE)中，对内存的寻址方式为
								# (段地址 << 4 + 偏移量) 可以寻址的线性空间为 20 位
	.equ ROOTDEV, 0x301			# 指定/dev/fda为系统镜像所在的设备

	ljmp    $BOOTSEG, $_start   # 修改cs寄存器为BOOTSEG, 并跳转到_start处执行我们的代码

_start:
	mov $BOOTSEG,%ax
	mov %ax, %es			# 设置好 ES 寄存器，为后续输出字符串准备
	mov	$0x03, %ah			# 在输出我们的信息前读取光标的位置, 会将光标当前所在行，列存储在DX里（DH为行, DL为列）
	xor	%bh, %bh
	int	$0x10
	
	mov	$20, %cx			# Set the output length
	mov	$0x0007, %bx		# page 0, color = 0x07 (from wikipedia https://en.wikipedia.org/wiki/INT_10H)
	mov     $msg1, %bp
	mov	$0x1301, %ax		# write string, move cursor
	int	$0x10				# 使用这个中断0x10的时候，输出的内容是从 ES:BP 中取得的，因而要设置好 ES 和 BP

load_demo:
	# 这里我们需要将软盘中的内容加载到内存中，并且跳转到相应地址执行代码
	mov $0x0000, %dx		# 选择磁盘号0，磁头号0进行读取
	mov $0x0002, %cx		# 从二号扇区，0轨道开始读(注意扇区是从1开始编号的)
	mov $DEMOSEG, %ax		# ES:BX 指向装载目的地址
	mov %ax, %es
	mov $0x0200, %bx		
	mov $02, %ah			# Service 2: Read Disk Sectors
	mov $4, %al				# 读取的扇区数
	int $0x13				# 调用BIOS中断读取
	jnc demo_load_ok		# 没有异常，加载成功
	jmp load_demo			# 并一直重试，直到加载成功

demo_load_ok:				# Here will jump to where the demo program is
	mov $DEMOSEG, %ax
	#mov %ax, %cs			# This is awful!! Do not change CS alone!! It will result in GDB cannot debug the code
							# And, of course the code will not work
	mov %ax, %ds
	ljmp $0x1020, $0		# jump to where the demo program exists

sectors:
	.word 0

msg1:
	.byte 13,10
	.ascii "Hello VOID001!"
	.byte 13,10,13,10

	.= 0x1fe                    # 这里是对齐语法 等价于 .org 表示在该处补零，一直补到 地址为 510 的地方 (即第一扇区的最后两字节)
								# 然后在这里填充好0xaa55魔术值，BIOS会识别硬盘中第一扇区以0xaa55结尾的为启动扇区，于是BIOS会装载
								# 代码并且运行
boot_flag:
	.word 0xAA55
