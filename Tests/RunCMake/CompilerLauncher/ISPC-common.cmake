enable_language(ISPC)
enable_language(CXX)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
add_executable(main main.cxx test.ispc)
