
enable_language(C)
add_executable(empty empty.c)
target_compile_definitions(empty PRIVATE $<HOST_LINK:DEF>)
