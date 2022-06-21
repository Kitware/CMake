enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "3c375311-a3c9-4396-a187-3227ef642046")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET UNKNOWN)
