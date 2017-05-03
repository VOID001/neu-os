# system_call.s 里面实现了系统调用的过程
# 实现了时钟中断和fork 必要的系统调用
# 还没有对信号进行处理

.global timer_interrupt, system_call, sys_fork

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

nr_system_calls = 72+1 #sys_debug

# offset in task_struct
state = 0
counter = 4

.align 2
bad_sys_call:
	movl $-1, %eax
	iret

.align 2
reschedule:
	pushl $ret_from_syscall
	jmp schedule

.align 2
system_call:
	cmp $nr_system_calls - 1, %eax
	ja bad_sys_call
	push %ds
	push %es
	push %fs
	push %edx
	push %ecx
	push %ebx
	movl $0x10, %edx
	mov %dx, %ds
	mov %dx, %es
	movl $0x17, %edx
	mov %dx, %fs
	call *sys_call_table(, %eax, 4)
	pushl %eax
	movl current,%eax		# 这里对task_struct进行判断，如果current->state != TASK_RUNNING, 则需要进行重新调度
	cmp $0, state(%eax)
	jne reschedule
	cmp $0, counter(%eax)	# 如果时间片用光，也需要重新调度
	je reschedule
ret_from_syscall:
# 这里暂时不处理信号
	popl %eax
	popl %ebx
	popl %ecx
	popl %edx
	pop %fs
	pop %es
	pop %ds
	iret

.align 2
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
	movl $0x17, %eax
	mov	%ax, %fs
	incl jiffies
	movb $0x20, %al
	outb %al, $0x20
	movl CS(%esp), %eax		# 将 CS 取出，并
	andl $3, %eax			# 计算出 CPL
	pushl %eax
	call do_timer
	addl $4, %esp
	jmp ret_from_syscall

.align 2
sys_fork:
	call find_empty_process
	testl %eax, %eax			# 检查返回值是不是负值, 负值意味着没有获得到pid资源
	js 1f
	push %gs
	pushl %esi
	pushl %edi
	pushl %ebp
	pushl %eax
	call copy_process
	addl $20, %esp
1: ret
