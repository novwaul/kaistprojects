### --------------------------------------------------------------------
### Assignment 4
### In Je Hwang, 20160788	
### mydc.s
### Desk Calculator (dc)
### --------------------------------------------------------------------

	.equ   ARRAYSIZE, 20
	.equ   EOF, -1
	.equ   q, 113
	.equ   p, 112
	.equ   f, 102
	.equ   c, 99
	.equ   d, 100
	.equ   r, 114
	.equ   true, 1
	.equ   OADDEND1, 8
	.equ   OADDEND2, 12
	## for + character
	.equ   plus, 43
	## for - character
	.equ   minus, 45
	## for * character
	.equ   mul, 42
	## for / character
	.equ   div, 47
	## for % character
	.equ   rem, 37
	## for ^ character
	.equ   exp, 94
	## for _ character
	.equ   undbar, 95
	## for null character
	.equ   null, 0
	

###--------------------------------------------------------------------

        .section ".rodata"
scanfFormat:
	.asciz "%s"
printFormat:
	.asciz "%d\n"
StackerrMessage:
	.asciz "dc: stack empty\n"

### --------------------------------------------------------------------

        .section ".data"

### --------------------------------------------------------------------

        .section ".bss"
buffer:
        .skip  ARRAYSIZE

### --------------------------------------------------------------------

	.section ".text"

	## -------------------------------------------------------------
	## int expf(int, int)
	## Runs exponent operation. Returns the result.
	## -------------------------------------------------------------
	.type   expf,@function
expf:
	pushl   %ebp
	movl    %esp, %ebp
	pushl   %ebx
	pushl   %esi
	movl    OADDEND1(%ebp), %ebx
        movl    OADDEND2(%ebp), %eax
	movl    OADDEND2(%ebp), %esi
	## put 1 in ecx to use it as counter
	movl    $1, %ecx
	## check exponent is 0
	cmpl    $0, %ebx
	je      set1
expfloop:
	## do operation
	cmpl    %ecx, %ebx
	je      endfunc
	imull   %esi
	addl    $1, %ecx
	jmp     expfloop
set1:	
	## make return value 1 if exponent is 0
	movl    $1, %eax
	jmp     endfunc
endfunc:
	popl    %esi
	popl    %ebx
	movl    %ebp, %esp
	popl    %ebp
	ret

	## -------------------------------------------------------------
	## int main(void)
	## Runs desk calculator program.  Returns 0.
	## -------------------------------------------------------------
	.globl  main
	.type   main,@function

main:

	pushl   %ebp
	movl    %esp, %ebp

input:

	## dc number stack initialized. %esp = %ebp
	
	## scanf("%s", buffer)
	pushl	$buffer
	pushl	$scanfFormat
	call    scanf
	addl    $8, %esp

	## check if user input EOF
	cmp	$EOF, %eax
	je	quit

	## get first character in the buffer
	movl    $0, %eax
	movb    buffer, %al
	
	## check if user input q
	cmpb    $q, %al
	je      quit

	## check if user input p
	cmpb    $p, %al
	jne     endP
	## check at least one element is in stack
	cmpl    %esp, %ebp
	je      printErr
	## do operation
	movl    (%esp), %eax
	pushl   %eax
	pushl   $printFormat
	call    printf
	addl    $8, %esp
	jmp     input
endP:	

	## check if user input +
	cmpb    $plus, %al
	jne     endPlus
	## check at least two elements are in stack
	cmpl    %esp, %ebp
	je      printErr
	leal    4(%esp), %ecx
	cmpl    %ecx, %ebp
	je      printErr
	## do operation
	popl    %ecx
	popl    %eax
	addl    %ecx, %eax
	pushl   %eax
	jmp     input
endPlus:
	
	## check if user input -
	cmpb    $minus, %al
	jne     endMinus
	## check at least two elements are in stack
	cmpl    %esp, %ebp
	je      printErr
	leal    4(%esp), %ecx
	cmpl    %ecx, %ebp
	je     printErr
	## do operation
	popl    %ecx
	popl    %eax
	subl    %ecx, %eax
	pushl   %eax
	jmp     input
endMinus:

	## check if user input *
	cmpb    $mul, %al
	jne     endMul
	## check at least two elements are in stack
	cmpl    %esp, %ebp
	je      printErr
	leal    4(%esp), %ecx
	cmpl    %ecx, %ebp
	je      printErr
	## do operation
	popl    %ecx
	popl    %eax
	imull   %ecx
	pushl   %eax
	jmp     input
endMul:

	## check if user input /
	cmpb    $div, %al
	jne     endDiv
	## check at least two elements are in stack
	cmpl    %esp, %ebp
	je      printErr
	leal    4(%esp), %ecx
	cmpl    %ecx, %ebp
	je      printErr
	## prevent division by 0
	movl    (%esp), %ecx
	cmpl    $0, %ecx
	je      input
	## do opertation
	popl    %ecx
	popl    %eax
	movl    %eax, %edx
	sarl    $31, %edx
	idivl   %ecx
	pushl   %eax
	jmp     input
endDiv:

	## check if user input %
	cmpb    $rem, %al
	jne     endRem
	## check at least two elements are in stack
	cmpl    %esp, %ebp
	je      printErr
	leal    4(%esp), %ecx
	cmpl    %ecx, %ebp
	je      printErr
	## prevent division by 0
	movl    (%esp), %ecx
	cmpl    $0, %ecx
	je      input
	## do operation
	popl    %ecx
	popl    %eax
	movl    %eax, %edx
	sarl    $31, %edx
	idivl   %ecx
	pushl   %edx
	jmp     input
endRem:

	## check if user input ^
	cmpb    $exp, %al
	jne     endExp
	## check at least two elements are in stack
	cmpl    %esp, %ebp
	je      printErr
	leal    4(%esp), %ebx
	cmpl    %ebx, %ebp
	je      printErr
	## prevent negative exponent
	cmpl    $0, (%esp)
	jl      input
	## do operation
	call    expf
	addl    $8, %esp
	pushl   %eax
	jmp     input
endExp:

	## check if user input f
	cmpb    $f, %al
	jne     endF
	## do operation
	movl    %esp, %ebx
floop:	
	cmpl    %ebx, %ebp
	je      input
	movl    (%ebx), %eax
	pushl   %eax
	pushl   $printFormat
	call    printf
	addl    $8, %esp
	## make ebx towards next value
	addl    $4, %ebx
	jmp     floop
endF:	

	## check if user input c
	cmpb    $c, %al
	jne     endC
cloop:
	## do operation
	cmpl    %esp, %ebp
	je      input
	popl    %eax
	jmp     cloop
endC:

	## check if user input d
	cmpb    $d, %al
	jne     endD
	## check at least one element is in stack
	cmpl    %esp, %ebp
	je      printErr
	## do operation
	movl    (%esp), %eax
	pushl   %eax
	jmp     input
endD:

	## check if user input r
	cmpb    $r, %al
	jne     endR
	## check at least two elements are in stack
	cmpl    %esp, %ebp
	je      printErr
	leal    4(%esp), %eax
	cmpl    %eax, %ebp
	je      printErr
	## do operation
	popl    %eax
	popl    %ecx
	pushl   %eax
	pushl   %ecx
	jmp     input
endR:

	## check argument is empty or not
	pushl   $buffer
	call    strlen
	addl    $4, %esp
	cmpl    $null, %eax
	je      input
	## move strlength to ebx
	movl    %eax, %ebx
	## set ecx 0 to use it as a index
	movl    $0, %ecx
checkloop:
	## do argument check operation
	cmpl    %ecx, %ebx
	je      checkend
	# check _ is in the argument
	movl    $buffer, %eax
	addl    %ecx, %eax
	movl    $0, %edx
	movb    (%eax), %dl
	cmpb    $undbar, %dl
	je      validsign
	# check character is digit or not
	pushl   %ecx
	pushl   %edx
	call    isdigit
	addl    $4, %esp
	popl    %ecx
	cmpl    $null, %eax
	je      input
	# go back to checkloop
	addl    $1, %ecx
	jmp     checkloop
validsign:
	## change _ character in string to - character
	# check _ appears in the right position
	cmpl    $0, %ecx
	jne     input
	# change _ to -
	movb    $minus, buffer
	addl    $1, %ecx
	jmp     checkloop
checkend:
	pushl   $buffer
	call    atoi
	addl    $4, %esp
	pushl   %eax
	jmp     input

printErr:
	## operates when error occurs
	pushl   $StackerrMessage
	call    printf
	addl    $4, %esp
	jmp     input

quit:
	## quit the program
	# return 0
	movl    $0, %eax
	movl    %ebp, %esp
	popl    %ebp
	ret
