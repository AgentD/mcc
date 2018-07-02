	.text
	.globl	_start
	.type	_start, @function
_start:
	finit
	call	main
	pushl	$0
	call	exit
	.size	_start, .-_start


	.globl	print
	.type	print, @function
print:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	8(%ebp)
	pushl	$.LC0
	call	printf
	leave
	ret
	.size	print, .-print


	.globl	print_nl
	.type	print_nl, @function
print_nl:
	pushl	$.LC1
	call	printf
	addl	$4, %esp
	ret
	.size	print_nl, .-print_nl


	.globl	print_int
	.type	print_int, @function
print_int:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	8(%ebp)
	pushl	$.LC2
	call	printf
	leave
	ret
	.size	print_int, .-print_int



	.globl	print_float
	.type	print_float, @function
print_float:
	pushl	%ebp
	movl	%esp, %ebp
	pushl	8(%ebp)
	pushl	$.LC3
	call	printf
	leave
	ret
	.size	print_float, .-print_float


	.globl	read_int
	.type	read_int, @function
read_int:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$4, %esp
	movl	$0, -4(%ebp)
	leal	-4(%ebp), %eax
	pushl	%eax
	pushl	$.LC2
	call	scanf
	movl	-4(%ebp), %eax
	leave
	ret
	.size	read_int, .-read_int


	.globl	read_float
	.type	read_float, @function
read_float:
	pushl	%ebp
	movl	%esp, %ebp
	subl	$8, %esp
	movl	$0, -8(%ebp)
	leal	-8(%ebp), %eax
	pushl	%eax
	pushl	$.LC3
	call	scanf
	flds	-8(%ebp)
	leave
	ret
	.size	read_float, .-read_float


	.section	.rodata
.LC0:
	.string	"%s"
.LC1:
	.string	"\n"
.LC2:
	.string	"%d"
.LC3:
	.string	"%f"
