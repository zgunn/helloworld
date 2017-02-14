%include 'functions.asm'

section .text
	global _start

_start:
	mov eax, 90	; move our first number into eax
	mov ebx, 9	; move our second number into ebx
	add eax, ebx	; add ebx to eax (also, sub eax, ebx and mul ebx (multiply eax by ebx))
	call iprintlf	; integer print with linefeed

	call quit
