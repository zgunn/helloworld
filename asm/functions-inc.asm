%include 'functions.asm'

section .data
	msg1 db 'Hello world!',	0h						; 0ah = '\n', 0h = '\0'
	msg2 db 'This is how we recycle in NASM', 0h

section .text
	global _start

_start:
	mov eax, msg1	; move the address of our first message string into eax
	call sprintlf		; call our string printing function

	mov eax, msg2	; move the address of our second string into eax
	call sprintlf		; call our string printing function

	call quit
