cmake_policy(SET CMP0104 NEW)
enable_language(CUDA)
set(CMAKE_VERBOSE_MAKEFILE TRUE)
add_executable(main main.cu)
