%include 'functions.asm'

section .text
	global _start

_start:
	mov	ecx, 0	; ecx is initialized to zero

nextNum:
	inc ecx	; increment ecx

	mov eax, ecx	; move the address of our integer into ecx
	add	eax, 48	; add 48 to our number to convert from integer to ascii for printing
	push eax	; push eax to the stack
	mov eax, esp	; get the address of the character on the stack
	call sprintlf	; call our print function

	pop eax	; clean up the stack so we don't have unneeded bytes taking up space
	cmp ecx, 10	; have we reached 10 yet? compare our counter to the decimal 10
	jne nextNum	; jump if not equal and keep counting

	call quit
