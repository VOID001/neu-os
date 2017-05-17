.global __sig_restore, __masksig_restore

__sig_restore:		# Use to restore to user routine
	addl $4, %esp
	popl %eax
	popl %ecx
	popl %edx
	popfl
	ret

__masksig_restore:
	addl $4, %esp
# We need a call here to set mask
	addl $4, %esp
	popl %eax
	popl %ecx
	popl %edx
	popfl
	ret
