enable_language(C)

add_library(lib1 STATIC empty.c)
target_sources(lib1 INTERFACE FILE_SET a TYPE HEADERS)
target_sources(lib1 PUBLIC FILE_SET a)
