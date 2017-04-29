.code32

# Code to implement low-level 80386 interrupt handler
# We'll handle int0 - int16 here, some interrupts will
# push addtional error code onto stack, so we need to implement two common
# handler

# 实现 Intel 80386 硬件相关中断，即中断号 0 - 16 的汇编处理程序
# 大部分函数都会去调用相应的 traps.c 中的 C 语言处理函数，汇编
# 只做最紧要的事情，主要流程在 C 函数里处理

# 下面仅对中断的具体含义进行简单的注释，想要了解详情的同学请查看 Intel IA32 手册 Vol 3A 第六章

.global divide_error, debug, nmi, int3, overflow, bounds, invalid_op
.global double_fault, coprocessor_segment_overrun
.global invalid_TSS, segment_not_present, stack_segment
.global general_protection, coprocessor_error, irq13, reserved
# 这两个是STUB的,之后会在system_call.s中实现
.global corprocessor_error, parallel_interrupt, device_not_available
.global demo_timer_interrupt

# 先处理无 error code 压栈的情况
# 相关中断有: 除零(Fault) 调试debug(Fault) nmi(Trap) 断点指令(Trap)

divide_error:
	pushl $do_divide_error			# 首先压栈要调用的函数
no_error_code:
	xchgl %eax, (%esp)				# 将函数地址交换到 %eax 中, (同时%eax入栈)
									# 将当前寄存器压栈
	pushl %ebx
	pushl %ecx
	pushl %edx
	pushl %edi
	pushl %esi
	pushl %ebp
	push %ds
	push %es
	push %fs
	pushl $0						# error code = 0
	lea 44(%esp), %edx				# 栈的地址由高地址向低地址生长，因而这里是去向栈底寻址
									# 我们刚刚 push 了 11 个参数，现在将指针回退到中断返回地址
									# 这一参数所在的栈指针位置, 并将地址放入 %edx 
	pushl %edx
	movl $0x10, %edx				# 初始化ds, es, fs加载为内核数据段选择符
	mov %dx, %ds
	mov %dx, %es
	mov %dx, %fs
	call *%eax						# 使用间接调用，调用 %eax 内存放的地址处的函数
	addl $8, %esp					# 丢弃最后入栈的两个参数， 让栈指针回到 %fs 入栈处
	pop %fs
	pop %es
	pop %ds
	popl %ebp
	popl %esi
	popl %edi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax						# 这里把原 eax 中内容恢复
	iret							# 中断返回，包括弹出相应的参数，以及返回原代码执行

# int1 debug 调试中断入口点 类型 Fault
debug:
	pushl $do_int3
	jmp no_error_code

# int2 Non maskable interrupts 入口点 类型 Trap
nmi:
	pushl $do_nmi
	jmp no_error_code

# int3 断点指令引起中断的入口点 类型 Trap
int3:
	pushl $do_int3
	jmp no_error_code

# int4 溢出错误入口点 类型 Trap
overflow:
	pushl $do_overflow
	jmp no_error_code

# int5 边界检查出错 类型 Fault
bounds:
	pushl $do_bounds
	jmp no_error_code

# int6 无效指令 类型 Fault
invalid_op:
	pushl $do_invalid_op
	jmp no_error_code

# int9 协处理器段超出 类型 Abort
coprocessor_segment_overrun:
	pushl $do_coprocessor_segment_overrun
	jmp no_error_code

# int15 其他 Intel 保留中断的入口点
reserved:
	pushl $do_reserved
	jmp no_error_code

# int45 -- irq13
# 协处理器处理完一个操作的时候就会发送 IRQ13 信号，通知CPU操作完成，80387执行计算时
# CPU 会等待其完成，下面通过写协处理端口(0xF0)消除BUSY信号，并重新激活80387的扩展请求
# 引脚 PERREQ

irq13:
	pushl %eax
	xorb %al, %al
	outb %al, $0xF0
	movb $0x20, %al
	outb %al, $0x20
	jmp 1f
1:	jmp 1f
1:	outb %al, $0xA0			# 向8259A 发送 EOI 信号
	popl %eax
	jmp coprocessor_error

# 下面的中断会在压入中断返回地址之后将出错号一同压栈，因此返回时需要弹出出错号

# Double Fault, 类型 Abort 有出错码
# 当CPU在调用一个异常处理程序的时候又检测到另一个异常，而且这两个异常无法被串行处理
# 
double_fault:
	pushl $do_double_fault
error_code:
	xchgl %eax, 4(%esp)		# 将出错号%eax交换，同时%eax入栈
	xchgl %ebx, (%esp)		# 将要调用的C函数的地址与%ebx交换，同时%ebx入栈
	pushl %ecx
	pushl %edx
	pushl %esi
	pushl %edi
	pushl %ebp
	pushl %ds
	pushl %es
	pushl %fs
	pushl %eax
	lea 44(%esp), %eax
	pushl %eax
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	call *%ebx				# 间接调用, %ebx 中存放的就是要调用的C函数的地址
	addl $8, %esp
	pop %fs
	pop %es
	pop %ds
	pop %ebp
	popl %edi
	popl %esi
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret

# int10 无效的任务状态段(TSS) 类型 Fault
invalid_TSS:
	pushl $do_invalid_TSS
	jmp error_code

# int11 段不存在 类型 Fault 
segment_not_present:
	pushl $do_segment_not_present
	jmp error_code

# int12 堆栈段错误 类型 Fault
stack_segment:
	pushl $do_stack_segment
	jmp error_code

# 一般保护性错误 类型 Fault
general_protection:
	pushl $do_general_protection
	jmp error_code

# coprocessor error 先在这里实现一个stub的，之后会实现
coprocessor_error:
	pushl $do_stub
	jmp error_code

parallel_interrupt: # 本版本没有实现，这里只发EOI
	pushl %eax
	movb $0x20, %al
	outb %al, $0x20
	popl %eax
	iret

device_not_available:
	pushl $do_stub
	jmp error_code

# int7 设备不存在 将在 kernel/system_call.s 中实现
# int14 页错误 将在 mm/page.s 中实现
# int16 协处理器错误 将在 kernel/system_call.s 中实现
# int 0x20 时钟中断 将在 kernel/system_call.s 中实现
# int 0x80 系统调用 将在 kernel/system_call.s 中实现
