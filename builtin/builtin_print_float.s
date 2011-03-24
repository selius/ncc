.data
.SL1:
	.string	"%f\n"
.text
.globl	__print_float
__print_float:
	push	%ebp
	mov	%esp, %ebp
	sub	$8, %esp
	flds	8(%ebp)
	fstpl	(%esp)
	push	$.SL1
	call	printf
	add	$12, %esp
	mov	%ebp, %esp
	pop	%ebp
	ret
.end
