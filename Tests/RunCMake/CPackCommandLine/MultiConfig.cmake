enable_language(C)

include(CPack)

add_library(foo foo.c)
set_property(TARGET foo PROPERTY DEBUG_POSTFIX _dbg)
set_property(TARGET foo PROPERTY RELEASE_POSTFIX _rel)

install(TARGETS foo)
