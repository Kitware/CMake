enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "2182bf5c-ef0d-489a-91da-49dbc3090d2a")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET a TYPE UNKNOWN)
