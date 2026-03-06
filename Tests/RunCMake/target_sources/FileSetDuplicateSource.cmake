enable_language(C)

add_library(lib1 STATIC empty.c)

# files are silently de-duplicated
target_sources(lib1 PRIVATE FILE_SET HEADERS FILES h1.h h1.h)
