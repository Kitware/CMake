enable_language(C)

add_library(lib1 INTERFACE)
target_sources(lib1 PUBLIC FILE_SET CXX_MODULES FILES lib1.c)
target_sources(lib1 PUBLIC FILE_SET SOURCES FILES lib2.c)
