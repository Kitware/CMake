enable_language(C)
add_library(emptyStatic STATIC empty.c)
add_library(emptyShared SHARED empty.c)
add_executable(main main.c)
