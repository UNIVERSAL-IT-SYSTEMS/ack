.define _brk
.define _sbrk
.extern _brk
.extern _sbrk
.text
_sbrk:		move.l nd,a0
		add.w  4(sp),a0
		move.w #0x11,d0
		trap #0
		bcs lcerror
		move.l nd,d0
		move.l d0,a0
		add.w  4(sp),a0
		move.l a0,nd
		rts
lcerror:	jmp cerror
_brk:		move.w #0x11,d0
		move.l 4(sp),a0
		trap #0
		bcs lcerror
		move.l 4(sp),nd
		clr.l d0
		rts
.data
nd:		.long endbss
.text
