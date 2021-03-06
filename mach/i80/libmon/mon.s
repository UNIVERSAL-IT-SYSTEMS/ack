.define .mon
.sect .text
.sect .rom
.sect .data
.sect .bss
.sect .text

! Monitor call
! Expects on stack:	monitor call number
!			parameters
! Implemented are the following monitor calls:
! number  1:	exit
! number  3:	read
! number  4:	write
! number  5:	open
! number  6:	close
! number 54:	ioctl
! If called with a number of a call that is not implemented,
! a trap is generated.

.mon:	pop h
	shld .retadr
	mov h,b
	mov l,c
	shld .bcreg

	pop h		! monitor call number
	mov a,l
	cpi 1
	jz monexit	! is it an exit?
	cpi 3
	jz monread	! is it a read?
	cpi 4
	jz monwrite	! is it a write?
	cpi 5
	jz monopen	! is it an open?
	cpi 6
	jz monclose	! is it a close?
	cpi 54
	jz monioctl
	jmp ebadmon	! trap

monexit:
	jmp .stop

monread:
	pop h		! file-descriptor, not used
	pop h		! hl = pointer to output buffer
	pop d		! de = number of bytes to be read
	lxi b,0		! bc will contain the number of bytes actually read
1:	mov a,d
	ora e
	jz 2f
	call getchar
	push psw
	call putchar	! echo character
	pop psw
	mov m,a
	inx h
	inx b
	dcx d
	cpi 0x0A	! is it a newline?
	jnz 1b
2:	push b
	lxi h,0
	push h
	jmp monret

monwrite:
	pop h		! file-descriptor, not used
	pop h		! hl = pointer to output buffer
	pop d		! de = number of bytes
	push d
1:	mov a,e
	ora d
	jz 2f
	mov a,m
	call putchar
	inx h
	dcx d
	jmp 1b

2:	push d		! no error
	jmp monret


monopen:
	pop h		! pointer to string
	pop h		! flag
	lxi h,-1
	push h		! push file descriptor
	push h		! push error code twice
	push h
	jmp monret

monclose:
	lxi h,0
	xthl		! remove file descriptor and push error code
	jmp monret

monioctl:
	pop h		! file descriptor
	pop h		! request
	lxi h,0
	xthl		! remove argp and push error code
	jmp monret

monret:
	lhld .bcreg
	mov b,h
	mov c,l
	lhld .retadr
	pchl
