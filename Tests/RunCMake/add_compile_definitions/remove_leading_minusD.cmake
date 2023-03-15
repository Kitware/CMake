
enable_language(C)

add_compile_definitions(-DDEF0 "$<1:-DDEF1>")

add_library(lib1 foo.c)
