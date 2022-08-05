enable_language(C)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
message(STATUS "CMAKE_C_COMPILER_ARG1='${CMAKE_C_COMPILER_ARG1}'")
add_executable(main main.c)
