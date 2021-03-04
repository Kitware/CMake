enable_language(C)
string(APPEND CMAKE_C_FLAGS " -TESTWX-")
add_executable(main main.c)
