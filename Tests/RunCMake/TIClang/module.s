	.text
	.syntax unified
	.section	.text.main,"ax",%progbits
	.hidden	main
	.globl	main
	.p2align	4
main:
	nop
	bx	lr
