.text

@ unsigned division function:
@ r0: numerator
@ r1: denominator
@ returns: r0 has the quotient
udiv:
	udiv r0, r0, r1
	mov pc, lr
	
@ unsigned mod function: obtains r0 modulo r1
@ r0: dividend
@ r1: modulo
@ returns: r0 has the modulo result
mod:
	udiv r2, r0, r1
	mls  r0, r1, r2, r0
	mov pc, lr
