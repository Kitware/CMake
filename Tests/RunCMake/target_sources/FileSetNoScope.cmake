enable_language(C)

add_library(lib1 STATIC empty.c)
target_sources(lib1 PRIVATE FILE_SET a TYPE HEADERS BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES h1.h)
set_property(TARGET lib1 PROPERTY HEADER_SETS)
target_sources(lib1 PRIVATE FILE_SET a TYPE HEADERS FILES h2.h)
