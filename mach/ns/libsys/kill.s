.define _kill
.sect .text
.sect .rom
.sect .data
.sect .bss
.sect .text
.extern _kill
.sect .text
_kill:
enter[], 0
movd 12(fp),tos
movd 8(fp),tos
movd 37,tos
jsr @.mon
cmpqd 0,tos
bne I0011
movd 0,r4
exit []
ret 0
I0011:
movd tos,r7
movd r7,@_errno
movd -1,r4
exit []
ret 0