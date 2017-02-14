; slen(string message)
; string len calc function
slen:
	push ebx
	mov	ebx, eax

nextchar:
	cmp byte [eax], 0
	jz finished
	inc	eax
	jmp nextchar

finished:
	sub eax, ebx
	pop ebx
	ret


; sprint(string message)
; string printing function
sprint:
	push edx
	push ecx
	push ebx
	push eax
	call slen

	mov edx, eax
	pop eax

	mov ecx, eax
	mov ebx, 1
	mov eax, 4
	int 80h

	pop ebx
	pop ecx
	pop edx
	ret


; void sprintlf(string message)
; string printing with linefeed function
sprintlf:
	call sprint

	push eax	; push eax onto the stack to preserve it while we use the eax register in this function
	mov eax, 0ah	; move 0ah into eax - 0ah is the ascii character for a linefeed ('\n')
	push eax	; push the linefeed character onto the stack so we can get the address
	mov eax, esp	; move the address of the current stack pointer into eax for sprint
	call sprint
	pop eax	; remove our linefeed character from the stack
	pop eax	; restore the original value of eax before our function was called
	ret	; return to our program


; void iprint(integer number)
; integer printing function
iprint:
	push eax	; preserve eax on the stack to be restored after function runs
	push ecx
	push edx
	push esi
	mov	ecx, 0	; counter of how many bytes we need to print in the end

divLoop:
	inc ecx	; count each byte to print - number of characters
	mov edx, 0	; empty edx
	mov esi, 10	; mov 10 into esi
	idiv esi	; divide eax by esi
	add edx, 48	; convert edx to its ascii representation - edx holds the remainder after a divide instruction
	push edx	; push edx (string representation of an interger) onto the stack
	cmp eax, 0	; can the register be divided any more?
	jnz	divLoop	; jump if not zero to the label divLoop

printLoop:
	dec	ecx	; count down each byte that we put on the stack
	mov eax, esp	; mov the stack pointer into eax for printing
	call sprint	; call our string print function
	pop eax	; remove last character from the stack to move esp forward
	cmp ecx, 0	; have we printed all bytes we pushed onto the stack?
	jnz	printLoop	; jump if not zero to label printLoop

	pop esi	; restore esi from the value we pushed onto the stack at the start
	pop edx
	pop ecx
	pop eax
	ret


; void iprintlf(integer number)
; integer printing function with linefeed (itoa)
iprintlf:
	call iprint	; call our integer printing function

	push eax	; push eax onto the stack to preserve it while we use the eax register in this function
	mov eax, 0ah	; move 0ah into eax, 0ah = '\n'
	push eax	; push the linefeed onto the stack so we can get the address
	mov eax, esp	; move the address of the current stack pointer into eax for sprint
	call sprint
	pop eax	; remove our linefeed character from the stack
	pop eax ; restore the original value of eax before our function was called
	ret


; int atoi(integer number)
; ascii to integer function (atoi)
atoi:
	push ebx	; preserve ebx on the stack to be restored after function runs
	push ecx
	push edx
	push esi
	mov esi, eax	; move pointer in eax into esi (our number to convert)
	mov eax, 0	; init eax with decimal value 0
	mov ecx, 0	; init ecx with decimal val 0

.multLoop:
	xor ebx, ebx	; resets both lower and upper bytes of ebx to be 0
	mov bl, [esi+ecx]	; move a single byte into ebx register's lower half
	cmp bl, 48	; compare ebx register's lower half value against ascii value 48 (char value 0)
	jl .finished	; jump if less than to label finished
	cmp bl, 57	; compare ebx register's lower half value against ascii value 57 (char value 9)
	jg .finished	; jump if greater than to label finished
	cmp bl, 10	; compare ebx register's lower half value against ascii value 10 (linefeed character)
	je .finished	; jump if equal to label finished
	cmp bl, 0	; compare ebx register's lower half value against ascii value 0 (end of string)
	jz .finished	; jump if zero to label finished

	sub bl, 48	; convert ebx register's lower half to decimal representation of ascii value
	add eax, ebx	; add ebx to our integer value in eax
	mov ebx, 10	; move decimal value 10 into ebx
	mul ebx	; multiply eax by ebx to get place value
	inc ecx	; increment ecx (our counter register)
	jmp .multLoop	; continue multiply loop


.finished:
	mov ebx, 10	; move decimal value 10 into ebx
	div	ebx	; divide eax by value in ebx (in this case 10)
	pop esi	; restore esi from the value we pushed onto the stack at the start
	pop edx
	pop ecx
	pop ebx
	ret

; void exit()
; exit program and restore resources
quit:
	mov ebx, 0
	mov eax, 1
	int 80h
	ret
