%include 'functions.asm'

section .text
	global _start

_start:
	pop ecx	; first value on the stack is the number of arguments
	mov edx, 0	; init our data register to store additions

nextArg:
	cmp ecx, 0h	; check to see if we have any args left
	jz endofargs	; if zero flag is set jump to endofargs label (jumping over end of loop)
	pop eax	; pop the next arg off the stack
	call atoi	; convert our ascii string to decimal integer
	add edx, eax	; perform our addition logic
	dec ecx	; decrement ecx (number of args left) by 1
	jmp nextArg	; jump to label nextArg

endofargs:
	mov eax, edx	; move our data result into eax for printing
	call iprintlf	; call our integer print with linefeed function
	call quit
