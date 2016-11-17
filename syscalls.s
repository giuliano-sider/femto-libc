.text

.global write

@ r0: file descriptor where writing will take place
@ r1: buffer with content to be written
@ r2: number of bytes from buffer to be written
@ returns: number of bytes actually written
write:
	push {r7, lr} @ yes, apparently the syscalls can clobber lr
	.equ syscall_write, 4 @ syscall number for write
	mov r7, #syscall_write
	svc 0x0
	pop {r7, pc}


	
