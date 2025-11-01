	.file	"main.c"

	.extern __imp_GetStdHandle
	.extern __imp_WriteConsoleA
	.extern __imp_ExitProcess

	.bss

	.globl	std_out
	.align 8
std_out:
	.space 8

	.globl	str
	.section .rdata,"r"
.LC0:
	.ascii "Hello world\12\0"
	.data
	.align 8
str:
	.quad	.LC0
	.globl	len
	.align 4
len:
	.long	12
	.text
	.globl	main
main:
	pushq	%rbp
	movq	%rsp, %rbp
	subq	$64, %rsp

	movl	$-11, %ecx
	movq	__imp_GetStdHandle(%rip), %rax
	call	*%rax
	movq	%rax, std_out(%rip)

	movq	std_out(%rip), %rax
	movq	%rax, %rcx
	movq	str(%rip), %rdx
	movl	len(%rip), %eax
	movl	%eax, %r8d
	movl	$0, %r9d
	movq	$0, 32(%rsp)
	movq	__imp_WriteConsoleA(%rip), %rax
	call	*%rax

	movl	$0, %ecx
	movq	__imp_ExitProcess(%rip), %rax
	call	*%rax
