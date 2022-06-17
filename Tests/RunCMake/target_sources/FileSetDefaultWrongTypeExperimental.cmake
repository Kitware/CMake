enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "17be90bd-a850-44e0-be50-448de847d652")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET UNKNOWN)
