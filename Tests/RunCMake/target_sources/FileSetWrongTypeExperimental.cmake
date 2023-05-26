enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "aa1f7df0-828a-4fcd-9afc-2dc80491aca7")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET a TYPE UNKNOWN)
