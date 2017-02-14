%include 'functions.asm'

section .data
	msg1 db ' remainder '	; message string to correctly output result

section .text
	global _start

_start:
	mov eax, 90	; move our first number into eax
	mov ebx, 9	; move our second number into ebx
	div ebx	; divide eax by ebx
	call iprint	; call our integer print function on the quotient
	mov eax, msg1	; move our message string into eax
	call sprint	; call our string print function
	mov eax, edx	; move our remainder into eax
	call iprintlf	; call our integer print function with linefeed

	call quit
