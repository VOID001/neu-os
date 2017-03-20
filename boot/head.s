#######################################################
#    Lesson 5: 设置最初的页表，进行系统初始化		  #
#    这里要注意,页表我们设置在了0x00000000 ,这会覆盖  #
#	 head.s的代码                                     #
#######################################################

.text

.global idt, gdt, pg_dir, tmp_floppy_area
pg_dir:						# 这里说明pg_dir要写在这里，最后会覆盖掉startup code
.global startup_32

startup_32:
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	lss stack_start, %esp		# Setup the Stack to stack_start, Change SS and %esp together
	call setup_idt
	call setup_gdt
	mov $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	mov %ax, %gs
	lss stack_start, %esp
	xorl %eax, %eax
1:	incl %eax
	movl %eax, 0x000000		#Compare the address 0 with 1MB
	cmpl %eax, 0x100000
	je 1b					# If A20 not enabled, loop forever


# 检查486协处理器是否存在

	movl %cr0, %eax
	andl $0x80000011, %eax
	orl	$2, %eax			# Set MP Bit
	movl %eax, %cr0
	call check_x87
	jmp after_page_tables


check_x87:
	fninit
	fstsw %ax
	cmpb $0, %al
	je 1f
	movl %cr0, %eax
	xorl $6, %eax
	movl %eax, %cr0
	ret
.align 2
1:	.byte 0xDB, 0xE4  # 287 协处理器码
	ret


setup_idt:
	# We fill all the IDT entries with a default entry
	# that will display a message when any interrupt is triggered
	
	lea ignore_int, %edx
	movl $0x00080000, %eax
	movw %dx, %ax
	mov $0x8E00, %dx
	lea idt, %edi

	mov $256, %cx
rp_sidt:					# Fill the IDT Table will default stub entry
	mov %eax, (%edi)
	mov %edx, 4(%edi)
	addl $8, %edi
	dec %cx
	jne rp_sidt
	lidt idt_descr			# Load IDT Register

setup_gdt:
	lgdt gdt_descr
	ret

# Make place for pg directory
.org 0x1000					# Align to 4KB Boundary
pg0:

.org 0x2000
pg1:

.org 0x3000
pg2:

.org 0x4000
pg3:

.org 0x5000

tmp_floppy_area:
	.fill 1024, 1, 0		# fill here with 1024 (1)Bytes of 0

after_page_tables:
	push $0
	push $0
	push $0
	pushl $L6
	pushl $_main
L6:
	jmp L6

int_msg:					# Message to display when interrupt happen
	.asciz "Unknown interrupt\n\r"

.align 2					# Align to 4Bytes

# This routine is used to print a default message when any interrupt comes

ignore_int:
	pushl %eax
	pushl %ecx
	pushl %edx
	push %ds
	push %es
	push %fs
	movl $0x10, %eax
	mov %ax, %ds
	mov %ax, %es
	mov %ax, %fs
	pushl $int_msg
	call _printk
	
	popl %eax
	pop %fs
	pop %es
	pop %ds
	popl %edx
	popl %ecx
	popl %eax
	iret

.align 2
setup_paging:
	movl $1024 * 5, %ecx		# We have 5 pages (one page pg_dir + 4 pages)
	xorl %eax, %eax
	xorl %edi, %edi
	cld;rep;stosl				
	
	# Setup Page Directory(Only 4)
	movl $pg0+7, pg_dir			# +7 Means set attribute present bit, r/w user
	movl $pg1+7, pg_dir+4		# -- -- ---
	movl $pg2+7, pg_dir+8		# -- -- ---
	movl $pg3+7, pg_dir+12		# -- -- ---
	
	# Then we fill the rest of the page table
	# We mapped the highest linear address to Phy Address 16MB
	movl $pg3 + 4092, %edi
	movl $0xfff007, %eax			# 7 means present, r/w user attribute

	std
1:	stosl
	subl $0x1000, %eax
	jge 1b

	# Set up the Page Dir register cr3
	xorl %eax, %eax
	movl %eax, %cr3

	# Then enable paging
	movl %cr0, %eax
	orl $0x80000000, %eax			# Set the paging bit
	movl %eax, %cr0					# ENABLE PAGING NOW!
	ret

.align 2
.word 0

idt_descr:
	.word 256*8 - 1			# Length in Bytes - 1
	.long idt				# Base

.align 2
.word 0

gdt_descr:
	.word 256*8 - 1
	.long gdt

gdt:
	# Empty Entry (FIRST ENTRY)
	.quad 0x0000000000000000
	# BaseAddress = 0x00000000
	# Limit = 0xfff
	# Granularity = 1 means 4KB Segment limit are 4KB unit
	# TYPE = 0xA Executable Read
	# DPL = 0x00 S = 1 P = 1
	# Code Segment
	.quad 0x00c09a0000000fff
	# BaseAddress = 0x00000000
	# Limit = 0xfff
	# Granularity = 1 means 4KB Segment limit are 4KB unit
	# TYPE = 0x2 Read/Write
	# DPL = 0x00 S = 1 P = 1
	# Data Segment
	.quad 0x00c0920000000fff
	# Temporaray
	.quad 0x0000000000000000
	.fill 252, 8, 0

