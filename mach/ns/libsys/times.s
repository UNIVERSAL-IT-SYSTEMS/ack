.define _times
.sect .text
.sect .rom
.sect .data
.sect .bss
.sect .text
.extern _times
.sect .text
_times:
enter[], 0
movd 8(fp),tos
movd 43,tos
jsr @.mon
exit []
ret 0