.PUBLIC _main

.SECTION .textf,TEXTF
.TYPE _main,function,.LFE1-_main

_main:
        .STACK _main = 4
        clrw ax
        ret
.LFE1:
