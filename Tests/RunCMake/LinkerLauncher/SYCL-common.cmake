set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
enable_language(SYCL)
set(CMAKE_VERBOSE_MAKEFILE TRUE)

add_executable(main main.sycl)
