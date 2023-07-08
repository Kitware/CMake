enable_language(C)

set(CMAKE_EXPERIMENTAL_CXX_MODULE_CMAKE_API "bf70d4b0-9fb7-465c-9803-34014e70d112")

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET UNKNOWN)
