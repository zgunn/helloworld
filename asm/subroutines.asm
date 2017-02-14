section .data
	msg	db	'Hello, brave new world!', 0ah

section .text
	global _start

_start:
	mov	eax, msg	;move address of our string into eax
	call strlen	;call our function to calculate length of string

	mov	edx, eax	;our function leaves the result in eax
	mov	ecx, msg	;this is all the same as before
	mov	ebx, 1
	mov	eax, 4
	int	80h

	mov	ebx, 0
	mov	eax, 1
	int	80h

strlen:	;this is our first function declaration
	push ebx	;push the value in ebx onto the stack to preserve it while we use ebx in this function
	mov	ebx, eax	;move the address in eax into ebx (both point to the same segment in memory)

nextchar:	;same as in strlen.asm
	cmp	byte [eax], 0
	jz	finished
	inc	eax
	jmp nextchar

finished:
	sub	eax, ebx
	pop	ebx	;pop the value on the stack back into ebx
	ret
