enable_language(CXX)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
message(STATUS "CMAKE_CXX_COMPILER_ARG1='${CMAKE_CXX_COMPILER_ARG1}'")
add_executable(main main.cxx)
