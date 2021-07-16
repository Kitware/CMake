add_library(lib1 STATIC ../empty.c)
target_sources(lib1 PRIVATE FILE_SET HEADERS BASE_DIRS . FILES ../h1.h)
