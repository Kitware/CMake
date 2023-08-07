enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "a816ed09-43d1-40e5-bc8c-1a2824ee194e")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET UNKNOWN)
