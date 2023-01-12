ifndef DEF_FOO
.err <DEF_FOO incorrectly not defined>
endif
ifndef DEF_BAR
.err <DEF_BAR incorrectly not defined>
endif
ifndef TESTx64
.386
.model flat, c
endif
.code
include <foo-proc.asm>
end
