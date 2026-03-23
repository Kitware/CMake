
enable_language(C)

add_library(foo1 STATIC lib1.c)
target_sources(foo1 PRIVATE FILE_SET h1 TYPE HEADERS FILES h1.h)
target_sources(foo1 PRIVATE FILE_SET h2 TYPE HEADERS FILES h1.h)

add_library(foo2 STATIC lib1.c h1.h)
target_sources(foo2 PRIVATE FILE_SET h1 TYPE HEADERS FILES h1.h)
