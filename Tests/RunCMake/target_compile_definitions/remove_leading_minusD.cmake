
enable_language(C)

add_library(lib1 foo.c)
target_compile_definitions(lib1 PRIVATE -DDEF0 "$<1:-DDEF1>")
target_compile_definitions(lib1 PUBLIC -DDEF2 "$<1:-DDEF3>")

add_library(lib2 foo.c)
target_link_libraries(lib2 PRIVATE lib1)
