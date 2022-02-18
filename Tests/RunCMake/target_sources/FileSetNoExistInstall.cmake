enable_language(C)

add_library(lib1 STATIC empty.c)
install(TARGETS lib1 FILE_SET a)
