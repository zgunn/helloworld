%include 'functions.asm'

section .text
	global _start

_start:
	mov ecx, 0

nextNum:
	inc ecx
	mov eax, ecx
	call iprintlf
	cmp ecx, 10
	jne nextNum

	call quit
