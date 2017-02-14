%include 'functions.asm'

section .text
	global _start

_start:
	pop ecx	; first value on the stack is the number of arguments

nextarg:
	cmp ecx, 0h	; check to see if we have any arguments left
	jz endofargs	; if zero flag is set jump to endofargs label (jumping over the end of the loop)
	pop eax	; pop the next argument off the stack
	call sprintlf	; call our print with linefeed function
	dec ecx	; decrement ecx (number of args left) by of
	jmp nextarg	; jump to nextarg label (beginning of loop)

endofargs:
	call quit
