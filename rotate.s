.intel_syntax noprefix

formatStr: .ascii "%d\n"

.globl ASMrotateBlock

ASMrotateBlock:
	mov	rax, 0
	
	# Square-block; do nothing
	mov 	rbx, 0x569A
	cmp 	rdi, rbx
	jne	.I_1
	
	mov	rax, rdi
	
	jmp 	.done

.I_1:
	# Line-block, case 1
	mov 	rbx, 0x159D
	cmp 	rdi, rbx
	jne	.I_2
	
	mov	rax, 0x4567
	
	jmp 	.done

.I_2:
	# Line-block, case 2
	mov 	rbx, 0x4567
	cmp 	rdi, rbx
	jne	.iterate
	
	mov	rax, 0x159D
	
	jmp 	.done

.iterate:
	
	mov	r14, rdi
	shr	rdi, 2
	
	mov	r15, rdi
	shr	rdi, 2
	
	shl	rax, 2
	and	r14, 3
	or	rax, r14
	
	shl	rax, 2
	not	r15
	and	r15, 3
	or	rax, r15
	
	test 	rdi, rdi
	jne	.iterate
	
.done:
	.att_syntax noprefix
	ret


