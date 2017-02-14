section .data
	msg	db	'This message string can be an arbitrary length!', 0ah	;this can now be modified without having to update elsewhere in the program

section .text
	global _start

_start:
	mov	ebx, msg	;move address of message string into ebx
	mov	eax, ebx	;move address in ebx into eax as well (both now point to same segment in memory)

nextchar:
	cmp	byte [eax], 0	;compare the byte pointed to by eax at this address against zero (Zero is an end of string delimeter)
	jz	finished	;jump (if the zero flagged has been set) to the point in the code labeled 'finished'
	inc eax	;increment the address in eax by one byte (if the zero flagged has not been set)
	jmp	nextchar	;jump to the point in the code labeled 'nextchar'

finished:
	sub	eax, ebx	;subtract the address in ebx from the address in eax
					;remember both registers started pointing to the same address (in line 15)
					;but eax has been incremented one byte for each character in the message string
					;when you subtract one memory address from another of the same type
					;the result is the number of segments between them - in this case the number of bytes

	mov	edx, eax	;eax now equals the number of bytes in our string (msg)
					;the rest of the code should be familiar now
	mov	ecx, msg	;move memory address of message string into ecx
	mov ebx, 1	;write to STDOUT file
	mov eax, 4	;invoke SYS_WRITE (kernel opcode 4)
	int 80h

	mov	ebx, 0	;return status 0
	mov eax, 1	;invoke SYS_EXIT (kernel opcode 1)
	int 80h
