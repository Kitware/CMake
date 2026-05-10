
enable_language(C)

add_library(foo1 STATIC lib1.c)
target_sources(foo1 PRIVATE FILE_SET s1 TYPE SOURCES FILES lib2.c)
target_sources(foo1 PRIVATE FILE_SET s2 TYPE SOURCES FILES lib2.c)

add_library(foo2 STATIC lib1.c lib2.c)
target_sources(foo2 PRIVATE FILE_SET s1 TYPE SOURCES FILES lib2.c)
