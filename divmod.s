.data
.align 4
division_by_zero_msg: 
	.ascii "attempted to divide by zero. aborting into infinite NOP loop."
division_by_zero_msg_end:
	.word 0

.text
.globl udiv
.globl umod
.align 4

/******************************* udiv *********************************/

@ unsigned division function:
@ r0: dividend
@ r1: divisor
@ returns: r0 has the quotient
/*division steps: (first check if divisor is zero)
1. move to the left, subtracting the divisor from the dividend at every step, 
   until we go negative, or the divisor carries from a shift. 
   then undo last op, and move to step2.
2. subtract divisor from dividend until it goes negative. undo op.
   move to the right. if the position indicator carries, done.
**/	
udiv:
	cmp r1, #0
	beq division_by_zero_exception
	cmp r1, #1 @ special case: division by 1 can return immediately
	moveq pc, lr @ otherwise our shifted multiple of divisor below would be corrupted
	
	mov r2, #1 @ position indicator (shifted multiple of divisor)
	mov r3, #0 @ quotient

step1:
	subs r0, r0, r1 @ substract shifted multiple of divisor from dividend
	blo step1wentnegative @ can't subtract anymore
	add r3, r3, r2 @ and add the shift multiple to quotient
	mov r2, r2, lsl #1 @ subtract a bigger multiple of the divisor next time
	movs r1, r1, lsl #1 @ divisor must shift as well
	bcc step1 @ when divisor carries, we can't move anymore
step1carried:
	mov r1, r1, lsr #1
	orr r1, r1, #(1<<31) @ undo the carry. go to step2.
	mov r2, r2, lsr #1 @ position indicator doesn't carry since divisor > 1
	b step2
step1wentnegative:
	add r0, r0, r1 @ undo the subtraction. go to step2moveright
	b step2moveright
step2:
	subs r0, r0, r1
	blo step2wentnegative
	add r3, r3, r2 @ add the shift multiple to the quotient
	b step2
step2wentnegative:
	add r0, r0, r1 @ undo the subtraction
step2moveright:
	mov r1, r1, lsr #1 @ move the divisor to the right for a more modest multiple
	movs r2, r2, lsr #1 @ move the shift multiple to the right
	bcc step2 @ if the shift multiple carries, it is zero: we can't divide anymore. done.

	mov r0, r3 @ return the value of the quotient
mov pc, lr

division_by_zero_exception:
	.equ stderr_fd, 2
	mov r0, #stderr_fd
	ldr r1, =division_by_zero_msg
	mov r2, #(division_by_zero_msg_end - division_by_zero_msg)
	bl write
division_by_zero_trap: @ stay here forever for debugging
	b division_by_zero_trap

/******************************* umod *******************************/

@ unsigned mod function: obtains r0 modulo r1
@ r0: dividend
@ r1: modulo
@ returns: r0 has the modulo result
umod:
push {r4, r5, lr}
	mov r5, r1 @ modulo/divisor saved here
	mov r4, r0 @ dividend saved here
	bl udiv @ obtain quotient
	mls  r0, r0, r5, r4 @ places (dividend - quotient*divisor) in r0
pop {r4, r5, pc}
