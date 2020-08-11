
enable_language(C)

add_library(lib empty.c)

add_executable(empty empty.c)
set_property(TARGET empty PROPERTY LINK_DEPENDS $<HOST_LINK:lib>)
