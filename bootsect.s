#################################################################
#                                                               #
#  Lesson 1: 创建一个"Hello World"引导扇区                      #
#  Goal: 了解操作系统引导的过程，并且通过使用实模式下的汇编     #
#  在屏幕上输出一行字符串并进入到死循环                         #
#                                                               #
#################################################################

	.code16        # 指定语法为 十六位汇编

# rewrite with AT&T syntax by falcon <wuzhangjin@gmail.com> at 081012
# Modified by VOID001<zhangjianqiu13@gmail.com> at 2017 03 05
# loads pretty fast by getting whole sectors at a time whenever possible.

	#.global _start, begtext, begdata, begbss, endtext, enddata, endbss
	.global _start # 程序开始处
	.text

	.equ BOOTSEG, 0x07c0		# 当此扇区被BIOS识别为启动扇区装载到内存中时，装载到0x07c0段处
								# 此时我们处于实模式(REAL MODE)中，对内存的寻址方式为
								# (段地址 << 4 + 偏移量) 可以寻址的线性空间为 20 位

	ljmp    $BOOTSEG, $_start   # 修改cs寄存器为BOOTSEG, 并跳转到_start处执行我们的代码

_start:
	mov $BOOTSEG,%ax
	mov %ax, %es			# 设置好 ES 寄存器，为后续输出字符串准备
	mov	$0x03, %ah			# 在输出我们的信息前读取光标的位置, 会将光标当前所在行，列存储在DX里（DH为行, DL为列）
	xor	%bh, %bh
	int	$0x10
	
	mov	$20, %cx			# Set the output length
	mov	$0x0007, %bx		# page 0, attribute 7 (normal) 设置必要的属性
	#lea	msg1, %bp
	mov     $msg1, %bp
	mov	$0x1301, %ax		# write string, move cursor
	int	$0x10				# 使用这个中断0x10的时候，输出的内容是从 ES:BP 中取得的，因而要设置好 ES 和 BP

loop_forever:
	jmp loop_forever

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
