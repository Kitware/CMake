.public _main

.section .text, text
_main:
    .stack _main = 0
    mov 0x00000000, r10
    jmp [r31]
