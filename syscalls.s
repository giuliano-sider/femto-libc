.text

.globl write
.globl read

@ r0: file descriptor where writing will take place
@ r1: buffer with content to be written
@ r2: number of bytes from buffer to be written
@ returns: number of bytes actually written
write:
push {r7, lr} @ yes, apparently the syscalls can clobber lr (in fact, they seem to overwrite the value of r7 as well, so don't count on r7 preserving the syscall number in between syscalls)
	.equ syscall_write, 4 @ syscall number for write
	mov r7, #syscall_write
	svc 0x0
pop {r7, pc}

@ r0: file descriptor from which reading will take place
@ r1: buffer to store content read
@ r2: maximum number of bytes to be read
@ returns: number of bytes actually read
read:
push {r7, lr}
	.equ syscall_read, 3 @ syscall number for read
	mov r7, #syscall_read
	svc 0x0
pop {r7, pc}

	
