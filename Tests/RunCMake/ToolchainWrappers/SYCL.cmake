set(CMAKE_EXPERIMENTAL_SYCL "c0d1fb10-2ece-420e-9d29-7d7f2b300f25")
include(common.cmake)
enable_language(SYCL)
example_exe(main.sycl)
