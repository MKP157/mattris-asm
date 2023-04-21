#############################################################################
# 
#			     ==== "rotate.s" ====
#	
#	Complete implementation of mattris' block-rotation process in
#	x86-64 assembly, specifically written in Intel MASM syntax.
#	This is for no particular reason, other than how I am more
#	familiar with MASM's syntax over AT&T.
#
#	Assumes that the only argument passed is a valid TetrInteger,
#	and rotates all of its coordinates using common 90-degree 
#	rotation (y,x)
#
#	Written by Matthew Kenneth Peterson; ID 3719754; github "MKP157"
#	
#	@ Dr. Jong-Kyou Kim, CS 2253 - Final Project
#	@ Last updated April 20, 2023
#
#############################################################################

.intel_syntax noprefix

.globl ASMrotateBlock

ASMrotateBlock:
	mov	rax, 0
	
# Square-block; do nothing
	mov 	rbx, 0x569A
	cmp 	rdi, rbx
	jne	.I_1
	
	mov	rax, rdi
	
	jmp 	.done

# Line-block, case 1 ( -- => | )
.I_1:
	mov 	rbx, 0x159D
	cmp 	rdi, rbx
	jne	.I_2
	
	mov	rax, 0x4567
	
	jmp 	.done

# Line-block, case 2 ( | => -- )
.I_2:
	mov 	rbx, 0x4567
	cmp 	rdi, rbx
	jne	.iterate
	
	mov	rax, 0x159D
	
	jmp 	.done

.iterate:
	
	mov	r14, rdi	# Copy rdi remainder to r14
	shr	rdi, 2		# Shift rdi right twice, removing last 2 bits (x)
	
	mov	r15, rdi	# Copy rdi remainder to r15
	shr	rdi, 2		# Shift rdi right twice, removing last 2 bits (y)
	
	shl	rax, 2		# Shift result left by 2 to make room for next variable
	and	r14, 3		# Mask last 2 bits of r14
	or	rax, r14	# Copy r14 into rax's end
	
	shl	rax, 2		# Shift result left by 2 to make room for next variable
	
	not	r15		# Bitwise-negate r15.
				# This is necessary, as coordinate rotation by 
				# 90deg stipulates (y,x) -> (x, -y)
	
	and	r15, 3		# Mask last 2 bits of r15
	or	rax, r15	# Copy r14 into rax's end
	
	test 	rdi, rdi	# Shortcut for comparing rdi to 0.
	jne	.iterate	# If rdi has not yet been expended, re-iterate.
	
.done:
	.att_syntax noprefix
	ret


