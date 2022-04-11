enable_language(C)

add_library(lib1 STATIC empty.c)
set_property(TARGET lib1 PROPERTY HEADER_SETS "a")
