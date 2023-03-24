	push	rbx		; save current block as a temp. block always rbx!
	
	test	rdi, rdi	; x param -> rdi. if x != 0, we want to print "[]"
	
	jne	.print0
	mov	rsi, 0x5b5d	; "[]"
	jmp	.L1
	
.print0:
	mov 	rsi, 0x202e	; " ."

.L1:
	mov	r14, rbx
	and	r14, 0x3
	shr	rbx, 3
	
	mov	r15, rbx
	and	r15, 0x3
	shr	rbx, 3
	
	; print!
	
	test	rbx, rbx
	jne	.L1
	
	pop	rbx
	; refresh the screen
	
