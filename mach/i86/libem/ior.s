.sect .text; .sect .rom; .sect .data; .sect .bss
.sect .text
.define	.ior

	! #bytes in cx
.ior:
	pop	bx		! return address
	mov	di,sp
	add	di,cx
	sar	cx,1
1:
	pop	ax
	or	ax,(di)
	stos
	loop	1b
	jmp	bx
