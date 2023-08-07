enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "ac01f462-0f5f-432a-86aa-acef252918a6")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET a TYPE UNKNOWN)
