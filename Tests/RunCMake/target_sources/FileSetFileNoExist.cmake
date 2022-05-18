enable_language(C)

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES noexist.h)
