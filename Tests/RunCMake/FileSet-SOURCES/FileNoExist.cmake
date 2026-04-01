enable_language(C)

add_library(lib1 STATIC lib1.c)
target_sources(lib1 PRIVATE FILE_SET SOURCES BASE_DIRS ${CMAKE_CURRENT_SOURCE_DIR} FILES noexist.c)
