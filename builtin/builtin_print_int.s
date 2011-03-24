.data
.SL1:
	.string	"%d\n"
.text
.globl	__print_int
__print_int:
	push	%ebp
	mov	%esp, %ebp
	push	8(%ebp)
	push	$.SL1
	call	printf
	add	$8, %esp
	mov	%ebp, %esp
	pop	%ebp
	ret
.end
