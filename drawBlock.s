.intel_syntax noprefix

.data
draw: .ascii "x"
erase: .ascii "."

.text

.globl _dbstart, _initscr, _refresh, _endwin, _getch, _exit, _mvprintw

_dbstart:
	mov	rbx, 0x4569	# T block for demo
	
	call 	initscr
	
	mov	rdi, 1
	call	drawBlock
	
	mov	rdi, 0
	call	drawBlock
	
	call	endwin
	call	exit
	
	mov 	rax, 60
	mov 	rdi, 0
	syscall
	
drawBlock:
	push	rbx		# save current block as a temp. block always rbx!
	
	test	rdi, rdi	# x param -> rdi. if x != 0, we want to print "[]"
	
	jne	.print0
	mov	rdx, (draw)
	jmp	.L1
	
.print0:
	mov 	rdx, (erase)

.L1:
	mov	r14, rbx
	and	r14, 0x3
	shr	rbx, 3
	
	mov	r15, rbx
	and	r15, 0x3
	shr	rbx, 3
	
	push 	rdi
	
	mov	rdi, r14	
	# param 1; y
	mov	rsi, r15	
	# param 2: x
	mov	rdx, rdx	
	# param 3: char/string
	mov 	rax, 0
	call	mvprintw
	
	pop 	rdi
	
	test	rbx, rbx
	jne	.L1
	
	call 	refresh
	call 	getch
	
	pop	rbx
	ret
	
.att_syntax noprefix


