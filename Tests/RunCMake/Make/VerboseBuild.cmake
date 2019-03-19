enable_language(C)

# Make sure compile command is not hidden in a temp file.
string(REPLACE "${CMAKE_START_TEMP_FILE}" "" CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")
string(REPLACE "${CMAKE_END_TEMP_FILE}" "" CMAKE_C_COMPILE_OBJECT "${CMAKE_C_COMPILE_OBJECT}")

add_executable(hello hello.c)
target_compile_definitions(hello PRIVATE "DEFINE_FOR_VERBOSE_DETECTION")
