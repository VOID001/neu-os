# system_call.s 里面实现了系统调用的过程
# 这里我们先仅仅实现时钟中断

.global timer_interrupt

# 我们来定义一下栈的布局

EAX = 0x00
EBX = 0x04
ECX = 0x08
EDX = 0x0C
FS = 0x10
ES = 0x14
DS = 0x18
EIP = 0x1C
CS = 0x20
EFLAGS = 0x24
OLDESP = 0x28
OLDSS = 0x2C

ret_from_syscall:
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx
	pop %fs
	pop %es
	pop %ds
	iret

timer_interrupt:
	push %ds
	push %es
	push %fs
	push %edx
	push %ecx
	push %ebx
	push %eax
	movl $0x10, %eax		# 让DS指向内核数据段
	mov %ax, %ds
	mov %ax, %es
	#movl $0x17, %eax
	#mov	%ax, %fs
	incl jiffies
	movb $0x20, %al
	outb %al, $0x20
	movl CS(%esp), %eax		# 将 CS 取出，并
	andl $3, %eax			# 计算出 CPL
	pushl %eax
	call do_timer
	addl $4, %esp
	jmp ret_from_syscall
