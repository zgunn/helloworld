section .data
	msg	db	'Hello World', 0ah ;assign msg variable with your string message

section .text
	global _start

_start:
	mov	edx, 13	;number of bytes to write, one for each letter plus 0ah (linefeed char '\n')
	mov	ecx, msg	;move memory address of message string into ecx
	mov	ebx, 1	;write to STDOUT file
	mov	eax, 4	;invoke SYS_WRITE (kernel opcode 4)
	int	80h

	mov	ebx, 0	;return 0 status on exit
	mov	eax, 1	;invoke SYS_EXIT (kernel opcode 1)
	int 80h
