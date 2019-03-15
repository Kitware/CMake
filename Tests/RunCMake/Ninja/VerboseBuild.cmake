enable_language(C)
add_executable(hello hello.c)
target_compile_definitions(hello PRIVATE "DEFINE_FOR_VERBOSE_DETECTION")
