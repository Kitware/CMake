; module.s: assembly "main" function returns 32.
;
        .name           "module.s"
        .text
        .align          2

        .globl          main
main:
        diab.li         r3,32
        blr
