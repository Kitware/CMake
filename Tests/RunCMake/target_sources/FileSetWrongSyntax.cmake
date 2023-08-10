enable_language(C)

add_library(lib1 STATIC)
target_sources(lib1 PRIVATE empty.c FILE_SET h1.h TYPE HEADERS)
