%ifndef DEF_FOO
%error "DEF_FOO incorrectly not defined"
%endif
section .text
%ifdef TEST2x64
global foo
%else
global _foo
%endif
;TASM compatibility mode allows 'include' instead of '%include'
include "foo-proc.asm"
