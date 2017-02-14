%include 'functions.asm'

section .data
	msg1 db 'Please enter your name: ', 0h
	msg2 db 'Hello, ', 0h

section .bss
	sinput: resb 255	; reserve a 255 byte space in memory for the user's input string

section .text
	global _start

_start:
	mov eax, msg1
	call sprint

	mov	edx, 255	; number of bytes to read
	mov ecx, sinput	; reserved space to store our input (known as a buffer)
	mov ebx, 0	; write to the STDIN file
	mov eax, 3	; invoke SYS_READ (kernel opcode 3)
	int 80h

	mov eax, msg2
	call sprint

	mov eax, sinput	; move our buffer into eax (note: input contains linefeed)
	call sprint	; call our print function

	call quit
