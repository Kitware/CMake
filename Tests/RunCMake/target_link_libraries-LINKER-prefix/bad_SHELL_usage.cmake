
enable_language(C)

add_library(example SHARED LinkOptionsLib.c)
target_link_libraries(example PRIVATE "LINKER:-foo,SHELL:-bar")
