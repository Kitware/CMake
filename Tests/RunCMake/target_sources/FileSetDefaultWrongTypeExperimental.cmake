enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "9629ab6c-6c0e-423f-bb9d-cc5ac4a22041")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET UNKNOWN)
