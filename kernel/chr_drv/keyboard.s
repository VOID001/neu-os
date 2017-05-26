# Very Simple Keyboard Program
# The whole char_drv is rewritten for teaching purpose
# Get keyboard interrupt then call the keyboard interrupt function

.text
.global keyboard_interrupt

keyboard_interrupt:
	pushl %eax
	pushl %ebx
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	xor %al, %al
# 读取扫描码
	inb $0x60, %al
	push %ax
# 发送 EOI 给8259
	movb $0x20, %al
	outb %al, $0x20
	call do_keyboard_interrupt
	pop  %ax
	pop  %es
	pop  %ds
	popl %edx
	popl %ecx
	popl %ebx
	popl %eax
	iret
